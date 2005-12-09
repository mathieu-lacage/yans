/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "tcp-bsd-connection.h"
#include "host.h"
#include "tcp-end-point.h"
#include "tag-ipv4.h"
#include "packet.h"
#include "chunk-tcp.h"
#include "ipv4-route.h"
#include "simulator.h"
#include "tcp.h"
#include "ipv4.h"
#include "tcp-pieces.h"
#include "chunk-piece.h"
#include "tcp-bsd-fsm.h"
#include "tcp-bsd.h"
#include "tcp-bsd-seq.h"


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#define TSTMP_GEQ(a,b)	((int)((a)-(b)) >= 0)
#define TSTMP_LT(a,b)       ((int)((a)-(b)) < 0)
#define TCP_PAWS_IDLE       (24 * 24 * 60 * 60 * PR_SLOWHZ)


#define TRACE_TCP_BSD_CONNECTION 1

#ifdef TRACE_TCP_BSD_CONNECTION
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP CONN " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_BSD_CONNECTION */
# define TRACE(format,...)
#endif /* TRACE_TCP_BSD_CONNECTION */



TcpBsdConnection::TcpBsdConnection ()
{
	m_tcp_keepidle = TCPTV_KEEP_IDLE;
	m_tcp_keepintvl = TCPTV_KEEPINTVL;
}
TcpBsdConnection::~TcpBsdConnection ()
{
	delete m_connection_completed;
	delete m_data_received;
	delete m_data_transmitted;
	delete m_ack_received;
	if (m_destroy != 0) {
		(*m_destroy) (this);
	}
	delete m_destroy;
}

void 
TcpBsdConnection::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
TcpBsdConnection::set_host (Host *host)
{
	m_host = host;
}
void 
TcpBsdConnection::set_end_point (TcpEndPoint *end_point)
{
	m_end_point = end_point;
	m_end_point->set_callback (make_callback (&TcpBsdConnection::receive, this));
}
void 
TcpBsdConnection::set_route (Route *route)
{
	m_route = route;
}
void 
TcpBsdConnection::set_destroy_handler (TcpBsdConnectionDestroy *handler)
{
	m_destroy = handler;
}

void 
TcpBsdConnection::set_callbacks (ConnectionCompletedCallback *connection_completed,
			      DataTransmittedCallback *data_transmitted,
			      DataReceivedCallback *data_received,
			      AckReceivedCallback *ack_received)
{
	m_connection_completed = connection_completed;
	m_data_received = data_received;
	m_data_transmitted = data_transmitted;
	m_ack_received = ack_received;
}

uint32_t 
TcpBsdConnection::get_room_left (void)
{
	return m_send->get_empty_at_back ();
}

uint32_t 
TcpBsdConnection::get_data_ready (void)
{
	return m_recv->get_data_at_front ();
}


uint32_t
TcpBsdConnection::send (Packet *packet)
{
	uint32_t sent = 0;
	ChunkPiece *piece = static_cast <ChunkPiece *> (packet->remove_header ());
	while (piece != 0) {
		ChunkPiece *copy = m_send->add_at_back (piece);
		if (copy == 0) {
			/* there was no room at all to put this piece
			 * in the buffer. */
			packet->add_header (piece);
			break;
		}
		sent += copy->get_size ();
		assert (copy->get_size () <= piece->get_size ());
		if (copy->get_size () < piece->get_size ()) {
			/* there was not enough room to put this
			 * piece entirely in the buffer. */
			piece->trim_start (copy->get_size ());
			packet->add_header (piece);
			break;
		}
		delete piece;
		piece = static_cast <ChunkPiece *> (packet->remove_header ());
	}
	notify_data_ready_to_send ();
	return sent;
}
Packet *
TcpBsdConnection::recv (uint32_t size)
{
	Packet *packet = m_recv->get_at_front (size);
	if (packet == 0) {
		return 0;
	}
	m_recv->remove_at_front (packet->get_size ());
	notify_room_ready_to_receive ();
	return packet;
}




void
TcpBsdConnection::drop (int errno)
{
        if (TCPS_HAVERCVDSYN(m_t_state)) {
                m_t_state = TCPS_CLOSED;
                // XXX (void) output(tp);
        }
        if (errno == ETIMEDOUT && m_t_softerror)
                errno = m_t_softerror;
        m_so_error = errno;
}

/*
 * Collect new round-trip time estimate
 * and update averages and current timeout.
 */
void
TcpBsdConnection::xmit_timer(short rtt)
{
	register short delta;

	if (m_t_srtt != 0) {
		/*
		 * srtt is stored as fixed point with 3 bits after the
		 * binary point (i.e., scaled by 8).  The following magic
		 * is equivalent to the smoothing algorithm in rfc793 with
		 * an alpha of .875 (srtt = rtt/8 + srtt*7/8 in fixed
		 * point).  Adjust rtt to origin 0.
		 */
		delta = rtt - 1 - (m_t_srtt >> TCP_RTT_SHIFT);
		if ((m_t_srtt += delta) <= 0)
			m_t_srtt = 1;
		/*
		 * We accumulate a smoothed rtt variance (actually, a
		 * smoothed mean difference), then set the retransmit
		 * timer to smoothed rtt + 4 times the smoothed variance.
		 * rttvar is stored as fixed point with 2 bits after the
		 * binary point (scaled by 4).  The following is
		 * equivalent to rfc793 smoothing with an alpha of .75
		 * (rttvar = rttvar*3/4 + |delta| / 4).  This replaces
		 * rfc793's wired-in beta.
		 */
		if (delta < 0)
			delta = -delta;
		delta -= (m_t_rttvar >> TCP_RTTVAR_SHIFT);
		if ((m_t_rttvar += delta) <= 0)
			m_t_rttvar = 1;
	} else {
		/* 
		 * No rtt measurement yet - use the unsmoothed rtt.
		 * Set the variance to half the rtt (so our first
		 * retransmit happens at 3*rtt).
		 */
		m_t_srtt = rtt << TCP_RTT_SHIFT;
		m_t_rttvar = rtt << (TCP_RTTVAR_SHIFT - 1);
	}
	m_t_rtt = 0;
	m_t_rxtshift = 0;

	/*
	 * the retransmit should happen at rtt + 4 * rttvar.
	 * Because of the way we do the smoothing, srtt and rttvar
	 * will each average +1/2 tick of bias.  When we compute
	 * the retransmit timer, we want 1/2 tick of rounding and
	 * 1 extra tick because of +-1/2 tick uncertainty in the
	 * firing of the timer.  The bias will give us exactly the
	 * 1.5 tick we need.  But, because the bias is
	 * statistical, we have to test that we don't drop below
	 * the minimum feasible timer (which is 2 ticks).
	 */
	TCPT_RANGESET(m_t_rxtcur, TCP_REXMTVAL,
		      m_t_rttmin, TCPTV_REXMTMAX);
	
	/*
	 * We received an ack for a packet that wasn't retransmitted;
	 * it is probably safe to discard any error indications we've
	 * received recently.  This isn't quite right, but close enough
	 * for now (a route might have failed after we sent a segment,
	 * and the return path might not be symmetrical).
	 */
	m_t_softerror = 0;
}

void 
TcpBsdConnection::canceltimers (void)
{
	register int i;

	for (i = 0; i < TCPT_NTIMERS; i++)
		m_t_timer[i] = 0;
}



void
TcpBsdConnection::receive (Packet *packet)
{
	caddr_t optp = NULL;
	int optlen;
	int len, tlen, off;
	int tiflags;
	int todrop, acked, ourfinisacked, needoutput = 0;
	short ostate;
	int dropsocket = 0;
	int iss = 0;
	u_long tiwin, ts_val, ts_ecr;
	int ts_present = 0;

	ChunkTcp *tcp = static_cast<ChunkTcp *> (packet->remove_header ());
	ChunkPiece *piece = new ChunkPiece ();
	piece->set_original (packet, 0, packet->get_size ());

	off = tcp->get_size ();
	tlen = packet->get_size ();
	tiflags = tcp->get_flags ();
	
	if (m_t_state == TCPS_CLOSED)
		goto drop;

	/* Unscale the window into a 32-bit value. */
	if ((tiflags & TH_SYN) == 0)
		tiwin = tcp->get_window_size () << m_snd_scale;
	else
		tiwin = tcp->get_window_size ();

	m_t_idle = 0;
	m_t_timer[TCPT_KEEP] = m_tcp_keepidle;

#if 0
	/*
	 * Process options if not in LISTEN state,
	 * else do it below (after getting remote address).
	 */
	if (optp && m_t_state != TCPS_LISTEN)
		tcp_dooptions(tp, optp, optlen, ti,
			&ts_present, &ts_val, &ts_ecr);
#endif


	/*
	 * Calculate amount of space in receive window,
	 * and then do TCP input processing.
	 * Receive window is amount of space in rcv queue,
	 * but not less than advertised window.
	 */
	{ 
		int win;
		win = m_recv->get_empty_at_back ();
		if (win < 0)
			win = 0;
		m_rcv_wnd = max(win, (int)(m_rcv_adv - m_rcv_nxt));
	}

	switch (m_t_state) {

	/*
	 * If the state is LISTEN then ignore segment if it contains an RST.
	 * If the segment contains an ACK then it is bad and send a RST.
	 * If it does not contain a SYN then it is not interesting; drop it.
	 * Don't bother responding if the destination was a broadcast.
	 * Otherwise initialize tp->rcv_nxt, and tp->irs, select an initial
	 * tp->iss, and send a segment:
	 *     <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK>
	 * Also initialize tp->snd_nxt to tp->iss+1 and tp->snd_una to tp->iss.
	 * Fill in remote peer address fields if not previously specified.
	 * Enter SYN_RECEIVED state, and process any other fields of this
	 * segment in this state.
	 */
	case TCPS_LISTEN: {
		if (tiflags & TH_RST)
			goto drop;
		if (tiflags & TH_ACK)
			goto dropwithreset;
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		/*
		 * RFC1122 4.2.3.10, p. 104: discard bcast/mcast SYN
		 * in_broadcast() should never return true on a received
		 * packet with M_BCAST not set.
		 */
		Ipv4Address ad = m_end_point->get_peer_address ();
		if (ad.is_multicast ())
			goto drop;
#if 0
		if (optp)
			tcp_dooptions(tp, optp, optlen, ti,
				&ts_present, &ts_val, &ts_ecr);
#endif
		if (iss)
			m_iss = iss;
		else
			m_iss = m_host->get_tcp ()->get_new_iss ();
		m_irs = tcp->get_sequence_number ();
		tcp_sendseqinit(this);
		tcp_rcvseqinit(this);
		m_t_flags |= TF_ACKNOW;
		m_t_state = TCPS_SYN_RECEIVED;
		m_t_timer[TCPT_KEEP] = TCPTV_KEEP_INIT;
		dropsocket = 0;		/* committed to socket */
		goto trimthenstep6;
	} break;

	/*
	 * If the state is SYN_SENT:
	 *	if seg contains an ACK, but not for our SYN, drop the input.
	 *	if seg contains a RST, then drop the connection.
	 *	if seg does not contain SYN, then drop it.
	 * Otherwise this is an acceptable SYN segment
	 *	initialize tp->rcv_nxt and tp->irs
	 *	if seg contains ack then advance tp->snd_una
	 *	if SYN has been acked change to ESTABLISHED else SYN_RCVD state
	 *	arrange for segment to be acked (eventually)
	 *	continue processing rest of data/controls, beginning with URG
	 */
	case TCPS_SYN_SENT:
		if ((tiflags & TH_ACK) &&
		    (SEQ_LEQ(tcp->get_ack_number (), m_iss) ||
		     SEQ_GT(tcp->get_ack_number (), m_snd_max)))
			goto dropwithreset;
		if (tiflags & TH_RST) {
			if (tiflags & TH_ACK)
				drop(ECONNREFUSED);
			goto drop;
		}
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		if (tiflags & TH_ACK) {
			m_snd_una = tcp->get_ack_number ();
			if (SEQ_LT(m_snd_nxt, m_snd_una))
				m_snd_nxt = m_snd_una;
		}
		m_t_timer[TCPT_REXMT] = 0;
		m_irs = tcp->get_sequence_number ();
		tcp_rcvseqinit(this);
		m_t_flags |= TF_ACKNOW;
		if (tiflags & TH_ACK && SEQ_GT(m_snd_una, m_iss)) {
			m_t_state = TCPS_ESTABLISHED;
#if 0
			/* Do window scaling on this connection? */
			if ((m_t_flags & (TF_RCVD_SCALE|TF_REQ_SCALE)) ==
				(TF_RCVD_SCALE|TF_REQ_SCALE)) {
				m_snd_scale = m_requested_s_scale;
				m_rcv_scale = m_request_r_scale;
			}
			(void) tcp_reass(tp, (struct tcpiphdr *)0,
				(struct mbuf *)0);
#endif
			/*
			 * if we didn't have to retransmit the SYN,
			 * use its rtt as our initial srtt & rtt var.
			 */
			if (m_t_rtt)
				xmit_timer (m_t_rtt);
		} else
			m_t_state = TCPS_SYN_RECEIVED;

trimthenstep6:
		/*
		 * Advance tcp->get_sequence_number () to correspond to first data byte.
		 * If data, trim to stay within window,
		 * dropping FIN if necessary.
		 */
		tcp->set_sequence_number (tcp->get_sequence_number () + 1);
		if (packet->get_size () > m_rcv_wnd) {
			todrop = packet->get_size () - m_rcv_wnd;
			piece->trim_end (todrop);
			tiflags &= ~TH_FIN;
		}
		m_snd_wl1 = tcp->get_sequence_number () - 1;
		m_rcv_up = tcp->get_sequence_number ();
		goto step6;
	}
	

	/*
	 * States other than LISTEN or SYN_SENT.
	 * First check timestamp, if present.
	 * Then check that at least some bytes of segment are within 
	 * receive window.  If segment begins before rcv_nxt,
	 * drop leading data (and SYN); if nothing left, just ack.
	 * 
	 * RFC 1323 PAWS: If we have a timestamp reply on this segment
	 * and it's less than ts_recent, drop it.
	 */
	if (ts_present && (tiflags & TH_RST) == 0 && m_ts_recent &&
	    TSTMP_LT(ts_val, m_ts_recent)) {

		/* Check to see if ts_recent is over 24 days old.  */
		if ((int)(m_host->get_tcp ()->now () - m_ts_recent_age) > TCP_PAWS_IDLE) {
			/*
			 * Invalidate ts_recent.  If this segment updates
			 * ts_recent, the age will be reset later and ts_recent
			 * will get a valid value.  If it does not, setting
			 * ts_recent to zero will at least satisfy the
			 * requirement that zero be placed in the timestamp
			 * echo reply when ts_recent isn't valid.  The
			 * age isn't reset until we get a valid ts_recent
			 * because we don't want out-of-order segments to be
			 * dropped when ts_recent is old.
			 */
			m_ts_recent = 0;
		} else {
			goto dropafterack;
		}
	}

	todrop = m_rcv_nxt - tcp->get_sequence_number ();
	if (todrop > 0) {
		if (tiflags & TH_SYN) {
			tiflags &= ~TH_SYN;
			tcp->set_sequence_number (tcp->get_sequence_number () + 1);
			tiflags &= ~TH_URG;
			todrop--;
		}
		if (todrop >= ((int)packet->get_size ())) {
			/*
			 * If segment is just one to the left of the window,
			 * check two special cases:
			 * 1. Don't toss RST in response to 4.2-style keepalive.
			 * 2. If the only thing to drop is a FIN, we can drop
			 *    it, but check the ACK or we will get into FIN
			 *    wars if our FINs crossed (both CLOSING).
			 * In either case, send ACK to resynchronize,
			 * but keep on processing for RST or ACK.
			 */
			if ((tiflags & TH_FIN && todrop == ((int)packet->get_size ()) + 1)
			    ) {
				todrop = packet->get_size ();
				tiflags &= ~TH_FIN;
				m_t_flags |= TF_ACKNOW;
			} else {
				/*
				 * Handle the case when a bound socket connects
				 * to itself. Allow packets with a SYN and
				 * an ACK to continue with the processing.
				 */
				if (todrop != 0 || (tiflags & TH_ACK) == 0)
					goto dropafterack;
			}
		}
		piece->trim_start (todrop);
		tcp->set_sequence_number (tcp->get_sequence_number () + todrop);
		tiflags &= ~TH_URG;
	}


	/*
	 * If segment ends after window, drop trailing data
	 * (and PUSH and FIN); if nothing left, just ACK.
	 */
	todrop = (tcp->get_sequence_number ()+piece->get_size ()) - (m_rcv_nxt+m_rcv_wnd);
	if (todrop > 0) {
		if (todrop >= ((int)piece->get_size ())) {
			/*
			 * If window is closed can only take segments at
			 * window edge, and have to drop data and PUSH from
			 * incoming segments.  Continue processing, but
			 * remember to ack.  Otherwise, drop segment
			 * and ack.
			 */
			if (m_rcv_wnd == 0 && tcp->get_sequence_number () == m_rcv_nxt) {
				m_t_flags |= TF_ACKNOW;
			} else
				goto dropafterack;
		}
		piece->trim_end (todrop);
		tiflags &= ~(TH_PUSH|TH_FIN);
	}

	/*
	 * If last ACK falls within this segment's sequence numbers,
	 * record its timestamp.
	 */
	if (ts_present && SEQ_LEQ(tcp->get_sequence_number (), m_last_ack_sent) &&
	    SEQ_LT(m_last_ack_sent, tcp->get_sequence_number () + piece->get_size () +
		   ((tiflags & (TH_SYN|TH_FIN)) != 0))) {
		m_ts_recent_age = m_host->get_tcp ()->now ();
		m_ts_recent = ts_val;
	}

	/*
	 * If the RST bit is set examine the state:
	 *    SYN_RECEIVED STATE:
	 *	If passive open, return to LISTEN state.
	 *	If active open, inform user that connection was refused.
	 *    ESTABLISHED, FIN_WAIT_1, FIN_WAIT2, CLOSE_WAIT STATES:
	 *	Inform user that connection was reset, and close tcb.
	 *    CLOSING, LAST_ACK, TIME_WAIT STATES
	 *	Close the tcb.
	 */
	if (tiflags&TH_RST) switch (m_t_state) {

	case TCPS_SYN_RECEIVED:
		m_so_error = ECONNREFUSED;
		goto close;

	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
		m_so_error = ECONNRESET;
	close:
		m_t_state = TCPS_CLOSED;
		goto drop;

	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:
		goto drop;
	}

	/*
	 * If a SYN is in the window, then this is an
	 * error and we send an RST and drop the connection.
	 */
	if (tiflags & TH_SYN) {
		drop (ECONNRESET);
		goto dropwithreset;
	}

	/*
	 * If the ACK bit is off we drop the segment and return.
	 */
	if ((tiflags & TH_ACK) == 0)
		goto drop;


	/*
	 * Ack processing.
	 */
	switch (m_t_state) {

	/*
	 * In SYN_RECEIVED state if the ack ACKs our SYN then enter
	 * ESTABLISHED state and continue processing, otherwise
	 * send an RST.
	 */
	case TCPS_SYN_RECEIVED:
		if (SEQ_GT(m_snd_una, tcp->get_ack_number ()) ||
		    SEQ_GT(tcp->get_ack_number (), m_snd_max))
			goto dropwithreset;
		m_t_state = TCPS_ESTABLISHED;
		/* Do window scaling? */
		if ((m_t_flags & (TF_RCVD_SCALE|TF_REQ_SCALE)) ==
			(TF_RCVD_SCALE|TF_REQ_SCALE)) {
			m_snd_scale = m_requested_s_scale;
			m_rcv_scale = m_request_r_scale;
		}
		// XXX
		//(void) tcp_reass(tp, (struct tcpiphdr *)0, (struct mbuf *)0);
		m_snd_wl1 = tcp->get_sequence_number () - 1;
		/* fall into ... */

	/*
	 * In ESTABLISHED state: drop duplicate ACKs; ACK out of range
	 * ACKs.  If the ack is in the range
	 *	tp->snd_una < ti->ti_ack <= tp->snd_max
	 * then advance tp->snd_una to ti->ti_ack and drop
	 * data from the retransmission queue.  If this ACK reflects
	 * more up to date window information we update our window information.
	 */
	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:

		if (SEQ_LEQ(tcp->get_ack_number (), m_snd_una)) {
			if (piece->get_size () == 0 && tiwin == m_snd_wnd) {
				/*
				 * If we have outstanding data (other than
				 * a window probe), this is a completely
				 * duplicate ack (ie, window info didn't
				 * change), the ack is the biggest we've
				 * seen and we've seen exactly our rexmt
				 * threshhold of them, assume a packet
				 * has been dropped and retransmit it.
				 * Kludge snd_nxt & the congestion
				 * window so we send only this one
				 * packet.
				 *
				 * We know we're losing at the current
				 * window size so do congestion avoidance
				 * (set ssthresh to half the current window
				 * and pull our congestion window back to
				 * the new ssthresh).
				 *
				 * Dup acks mean that packets have left the
				 * network (they're now cached at the receiver) 
				 * so bump cwnd by the amount in the receiver
				 * to keep a constant cwnd packets in the
				 * network.
				 */
				if (m_t_timer[TCPT_REXMT] == 0 ||
				    tcp->get_ack_number () != m_snd_una)
					m_t_dupacks = 0;
				else if (++m_t_dupacks == m_host->get_tcp ()->get_rexmtthresh ()) {
					tcp_seq onxt = m_snd_nxt;
					u_int win =
					    min(m_snd_wnd, m_snd_cwnd) / 2 /
						m_t_maxseg;

					if (win < 2)
						win = 2;
					m_snd_ssthresh = win * m_t_maxseg;
					m_t_timer[TCPT_REXMT] = 0;
					m_t_rtt = 0;
					m_snd_nxt = tcp->get_ack_number ();
					m_snd_cwnd = m_t_maxseg;
					// XXX
					//(void) tcp_output(tp);
					m_snd_cwnd = m_snd_ssthresh +
					       m_t_maxseg * m_t_dupacks;
					if (SEQ_GT(onxt, m_snd_nxt))
						m_snd_nxt = onxt;
					goto drop;
				} else if (m_t_dupacks > m_host->get_tcp ()->get_rexmtthresh ()) {
					m_snd_cwnd += m_t_maxseg;
					//XXX
					//(void) tcp_output(tp);
					goto drop;
				}
			} else
				m_t_dupacks = 0;
			break;
		}
		/*
		 * If the congestion window was inflated to account
		 * for the other side's cached packets, retract it.
		 */
		if (m_t_dupacks > m_host->get_tcp ()->get_rexmtthresh () &&
		    m_snd_cwnd > m_snd_ssthresh)
			m_snd_cwnd = m_snd_ssthresh;
		m_t_dupacks = 0;
		if (SEQ_GT(tcp->get_ack_number (), m_snd_max)) {
			goto dropafterack;
		}
		acked = tcp->get_ack_number () - m_snd_una;

		/*
		 * If we have a timestamp reply, update smoothed
		 * round trip time.  If no timestamp is present but
		 * transmit timer is running and timed sequence
		 * number was acked, update smoothed round trip time.
		 * Since we now have an rtt measurement, cancel the
		 * timer backoff (cf., Phil Karn's retransmit alg.).
		 * Recompute the initial retransmit timer.
		 */
		if (ts_present)
			xmit_timer(m_host->get_tcp ()->now ()-ts_ecr+1);
		else if (m_t_rtt && SEQ_GT(tcp->get_ack_number (), m_t_rtseq))
			xmit_timer(m_t_rtt);

		/*
		 * If all outstanding data is acked, stop retransmit
		 * timer and remember to restart (more output or persist).
		 * If there is more data to be acked, restart retransmit
		 * timer, using current (possibly backed-off) value.
		 */
		if (tcp->get_ack_number () == m_snd_max) {
			m_t_timer[TCPT_REXMT] = 0;
			needoutput = 1;
		} else if (m_t_timer[TCPT_PERSIST] == 0)
			m_t_timer[TCPT_REXMT] = m_t_rxtcur;
		/*
		 * When new data is acked, open the congestion window.
		 * If the window gives us less than ssthresh packets
		 * in flight, open exponentially (maxseg per packet).
		 * Otherwise open linearly: maxseg per window
		 * (maxseg^2 / cwnd per packet), plus a constant
		 * fraction of a packet (maxseg/8) to help larger windows
		 * open quickly enough.
		 */
		{
		register u_int cw = m_snd_cwnd;
		register u_int incr = m_t_maxseg;

		if (cw > m_snd_ssthresh)
			incr = incr * incr / cw + incr / 8;
		m_snd_cwnd = min(cw + incr, (u_int)TCP_MAXWIN<<m_snd_scale);
		}
		if (acked > ((int)m_send->get_data_at_front ())) {
			m_send->remove_at_front (m_send->get_data_at_front ());
			ourfinisacked = 1;
		} else {
			m_send->remove_at_front (acked);
			m_snd_wnd -= acked;
			ourfinisacked = 0;
		}
		(*m_data_transmitted) ();
		m_snd_una = tcp->get_ack_number ();
		if (SEQ_LT(m_snd_nxt, m_snd_una))
			m_snd_nxt = m_snd_una;

		switch (m_t_state) {

		/*
		 * In FIN_WAIT_1 STATE in addition to the processing
		 * for the ESTABLISHED state if our FIN is now acknowledged
		 * then enter FIN_WAIT_2.
		 */
		case TCPS_FIN_WAIT_1:
			if (ourfinisacked) {
				/*
				 * If we can't receive any more
				 * data, then closing user can proceed.
				 * Starting the timer is contrary to the
				 * specification, but if we don't get a FIN
				 * we'll hang forever.
				 */
				m_t_state = TCPS_FIN_WAIT_2;
			}
			break;

	 	/*
		 * In CLOSING STATE in addition to the processing for
		 * the ESTABLISHED state if the ACK acknowledges our FIN
		 * then enter the TIME-WAIT state, otherwise ignore
		 * the segment.
		 */
		case TCPS_CLOSING:
			if (ourfinisacked) {
				m_t_state = TCPS_TIME_WAIT;
				canceltimers ();
				m_t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			}
			break;

		/*
		 * In LAST_ACK, we may still be waiting for data to drain
		 * and/or to be acked, as well as for the ack of our FIN.
		 * If our FIN is now acknowledged, delete the TCB,
		 * enter the closed state and return.
		 */
		case TCPS_LAST_ACK:
			if (ourfinisacked) {
				goto drop;
			}
			break;

		/*
		 * In TIME_WAIT state the only thing that should arrive
		 * is a retransmission of the remote FIN.  Acknowledge
		 * it and restart the finack timer.
		 */
		case TCPS_TIME_WAIT:
			m_t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			goto dropafterack;
		}
	}

step6:
	/*
	 * Update window information.
	 * Don't look at window if no ACK: TAC's send garbage on first SYN.
	 */
	if ((tiflags & TH_ACK) &&
	    (SEQ_LT(m_snd_wl1, tcp->get_sequence_number ()) || m_snd_wl1 == tcp->get_sequence_number () &&
	    (SEQ_LT(m_snd_wl2, tcp->get_ack_number ()) ||
	     m_snd_wl2 == tcp->get_ack_number () && tiwin > m_snd_wnd))) {
		/* keep track of pure window updates */
		m_snd_wnd = tiwin;
		m_snd_wl1 = tcp->get_sequence_number ();
		m_snd_wl2 = tcp->get_ack_number ();
		if (m_snd_wnd > m_max_sndwnd)
			m_max_sndwnd = m_snd_wnd;
		needoutput = 1;
	}

	/*
	 * If no out of band data is expected,
	 * pull receive urgent pointer along
	 * with the receive window.
	 */
	if (SEQ_GT(m_rcv_nxt, m_rcv_up))
		m_rcv_up = m_rcv_nxt;


dodata:							/* XXX */
	
	/*
	 * Process the segment text, merging it into the TCP sequencing queue,
	 * and arranging for acknowledgment of receipt if necessary.
	 * This process logically involves adjusting tp->rcv_wnd as data
	 * is presented to the user (this happens in tcp_usrreq.c,
	 * case PRU_RCVD).  If a FIN has already been received on this
	 * connection then we just ignore the text.
	 */
	if ((piece->get_size () || (tiflags&TH_FIN)) &&
	    TCPS_HAVERCVDFIN(m_t_state) == 0) {
		//TCP_REASS(tp, ti, m, so, tiflags);
		/*
		 * Note the amount of data that peer has sent into
		 * our window, in order to estimate the sender's
		 * buffer size.
		 */
		// XXX
		//len = so->so_rcv.sb_hiwat - (m_rcv_adv - m_rcv_nxt);
	} else {
		//m_freem(m);
		tiflags &= ~TH_FIN;
	}

	/*
	 * If FIN is received ACK the FIN and let the user know
	 * that the connection is closing.
	 */
	if (tiflags & TH_FIN) {
		if (TCPS_HAVERCVDFIN(m_t_state) == 0) {
			m_t_flags |= TF_ACKNOW;
			m_rcv_nxt++;
		}
		switch (m_t_state) {

	 	/*
		 * In SYN_RECEIVED and ESTABLISHED STATES
		 * enter the CLOSE_WAIT state.
		 */
		case TCPS_SYN_RECEIVED:
		case TCPS_ESTABLISHED:
			m_t_state = TCPS_CLOSE_WAIT;
			break;

	 	/*
		 * If still in FIN_WAIT_1 STATE FIN has not been acked so
		 * enter the CLOSING state.
		 */
		case TCPS_FIN_WAIT_1:
			m_t_state = TCPS_CLOSING;
			break;

	 	/*
		 * In FIN_WAIT_2 state enter the TIME_WAIT state,
		 * starting the time-wait timer, turning off the other 
		 * standard timers.
		 */
		case TCPS_FIN_WAIT_2:
			m_t_state = TCPS_TIME_WAIT;
			canceltimers ();
			m_t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			break;

		/*
		 * In TIME_WAIT state restart the 2 MSL time_wait timer.
		 */
		case TCPS_TIME_WAIT:
			m_t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			break;
		}
	}

	/*
	 * Return any desired output.
	 */
	if (needoutput || (m_t_flags & TF_ACKNOW))
		//XXX(void) tcp_output(tp);
	return;

dropafterack:
	/*
	 * Generate an ACK dropping incoming segment if it occupies
	 * sequence space, where the ACK reflects our state.
	 */
	if (tiflags & TH_RST)
		goto drop;
	//m_freem(m);
	m_t_flags |= TF_ACKNOW;
	//(void) tcp_output(tp);
	//XXX
	return;

dropwithreset:
#if 0
	/*
	 * Generate a RST, dropping incoming segment.
	 * Make ACK acceptable to originator of segment.
	 * Don't bother to respond if destination was broadcast/multicast.
	 */
	if ((tiflags & TH_RST) || m->m_flags & (M_BCAST|M_MCAST) ||
	    IN_MULTICAST(ti->ti_dst.s_addr))
		goto drop;
	if (tiflags & TH_ACK)
		tcp_respond(tp, ti, m, (tcp_seq)0, ti->ti_ack, TH_RST);
	else {
		if (tiflags & TH_SYN)
			ti->ti_len++;
		tcp_respond(tp, ti, m, ti->ti_seq+ti->ti_len, (tcp_seq)0,
		    TH_RST|TH_ACK);
	}
	/* destroy temporarily created socket */
	if (dropsocket)
		(void) soabort(so);
	#endif
	return;

drop:
#if 0
	/*
	 * Drop space held by incoming segment and return.
	 */
	m_freem(m);
	/* destroy temporarily created socket */
	if (dropsocket)
		(void) soabort(so);
#endif
	return;
}


void
TcpBsdConnection::start_connect (void)
{}

void 
TcpBsdConnection::slow_timer (void)
{}
void 
TcpBsdConnection::fast_timer (void)
{}



void
TcpBsdConnection::notify_data_ready_to_send (void)
{}

void
TcpBsdConnection::notify_room_ready_to_receive (void)
{}


