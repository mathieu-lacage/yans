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
#include "tcp-bsd.h"
#define MAX_TCPOPTLEN	32	/* max # bytes that go in options */
#define TCPOUTFLAGS 1
#include "tcp-bsd-fsm.h"
#undef TCPOUTFLAGS
#include "tcp-bsd-seq.h"
#include "network-interface.h"


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define panic(x)

#define TCPIPHDRLEN (32)
#define IPHDRLEN (16)
#define    MCLBYTES        (1024)
#define    MCLOFSET        (MCLBYTES - 1)
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

int     tcp_backoff[TCP_MAXRXTSHIFT + 1] =
	{ 1, 2, 4, 8, 16, 32, 64, 64, 64, 64, 64, 64, 64 };


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
	m_end_point->set_callback (make_callback (&TcpBsdConnection::input, this));
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

u_int
TcpBsdConnection::mss (u_int offer)
{
	register int mss;
	u_long bufsize;
	//extern int tcp_mssdflt;

	

	{
		mss = m_route->get_interface ()->get_mtu () - TCPIPHDRLEN;
#if	(MCLBYTES & (MCLBYTES - 1)) == 0
		if (mss > MCLBYTES)
			mss &= ~(MCLBYTES-1);
#else
		if (mss > MCLBYTES)
			mss = mss / MCLBYTES * MCLBYTES;
#endif
#if 0
		if (!in_localaddr(inp->inp_faddr))
			mss = min(mss, tcp_mssdflt);
#endif
	}
	/*
	 * The current mss, t_maxseg, is initialized to the default value.
	 * If we compute a smaller value, reduce the current mss.
	 * If we compute a larger value, return it for use in sending
	 * a max seg size option, but don't store it for use
	 * unless we received an offer at least that large from peer.
	 * However, do not accept offers under 32 bytes.
	 */
	if (offer)
		mss = min(mss, ((int)offer));
	mss = max(mss, 32);		/* sanity */
	if (mss < m_t_maxseg || offer != 0) {
		/*
		 * If there's a pipesize, change the socket buffer
		 * to that size.  Make the socket buffers an integral
		 * number of mss units; if the mss is larger than
		 * the socket buffer, decrease the mss.
		 */
		bufsize = m_send->get_size ();
		if (((int)bufsize) < mss)
			mss = bufsize;
		m_t_maxseg = mss;

	}
	m_snd_cwnd = mss;

	return (mss);
}

void
TcpBsdConnection::dooptions(ChunkTcp *tcp, int *ts_present, u_long *ts_val, u_long *ts_ecr)
{
	if (tcp->is_option_mss () && 
	    tcp->is_flag_syn ()) {
		mss (tcp->get_option_mss ());
	}
	if (tcp->is_option_windowscale () &&
	    tcp->is_flag_syn ()) {
		m_t_flags |= TF_RCVD_SCALE;
		m_requested_s_scale = min(tcp->get_option_windowscale (), TCP_MAX_WINSHIFT);
	}
	if (tcp->is_option_timestamp ()) {
		*ts_present = 1;
		*ts_val = tcp->get_option_timestamp_value ();
		*ts_ecr = tcp->get_option_timestamp_reply ();
		if (tcp->is_flag_syn ()) {
			m_t_flags |= TF_RCVD_TSTMP;
			m_ts_recent = *ts_val;
			m_ts_recent_age = m_host->get_tcp ()->now ();
		}
	}
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
TcpBsdConnection::setpersist (void)
{
	register int t = ((m_t_srtt >> 2) + m_t_rttvar) >> 1;

	if (m_t_timer[TCPT_REXMT])
		panic("tcp_output REXMT");
	/*
	 * Start/restart persistance timer.
	 */
	TCPT_RANGESET(m_t_timer[TCPT_PERSIST],
		      t * ::tcp_backoff[m_t_rxtshift],
		      TCPTV_PERSMIN, TCPTV_PERSMAX);
	if (m_t_rxtshift < TCP_MAXRXTSHIFT)
		m_t_rxtshift++;
}


int
TcpBsdConnection::output (void)
{
	register long len, win;
	int off, flags;
	int idle, sendalot;

	/*
	 * Determine length of data that should be transmitted,
	 * and flags that will be used.
	 * If there is some data or critical controls (SYN, RST)
	 * to send, then transmit; otherwise, investigate further.
	 */
	idle = (m_snd_max == m_snd_una);
	if (idle && m_t_idle >= m_t_rxtcur)
		/*
		 * We have been idle for "a while" and no acks are
		 * expected to clock out any data we send --
		 * slow start to get ack "clock" running again.
		 */
		m_snd_cwnd = m_t_maxseg;

again:
	sendalot = 0;
	off = m_snd_nxt - m_snd_una;
	win = min(m_snd_wnd, m_snd_cwnd);

	flags = tcp_outflags[m_t_state];
	/*
	 * If in persist timeout with window of 0, send 1 byte.
	 * Otherwise, if window is small but nonzero
	 * and timer expired, we will send what we can
	 * and go to transmit state.
	 */
	if (m_t_force) {
		if (win == 0) {
			/*
			 * If we still have some data to send, then
			 * clear the FIN bit.  Usually this would
			 * happen below when it realizes that we
			 * aren't sending all the data.  However,
			 * if we have exactly 1 byte of unset data,
			 * then it won't clear the FIN bit below,
			 * and if we are in persist state, we wind
			 * up sending the packet without recording
			 * that we sent the FIN bit.
			 *
			 * We can't just blindly clear the FIN bit,
			 * because if we don't have any more data
			 * to send then the probe will be the FIN
			 * itself.
			 */
			if (off < ((int)m_send->get_data_at_front ()))
				flags &= ~TH_FIN;
			win = 1;
		} else {
			m_t_timer[TCPT_PERSIST] = 0;
			m_t_rxtshift = 0;
		}
	}
	len = min(((int)m_send->get_data_at_front ()), win) - off;

	if (len < 0) {
		/*
		 * If FIN has been sent but not acked,
		 * but we haven't been called to retransmit,
		 * len will be -1.  Otherwise, window shrank
		 * after we sent into it.  If window shrank to 0,
		 * cancel pending retransmit and pull snd_nxt
		 * back to (closed) window.  We will enter persist
		 * state below.  If the window didn't close completely,
		 * just wait for an ACK.
		 */
		len = 0;
		if (win == 0) {
			m_t_timer[TCPT_REXMT] = 0;
			m_snd_nxt = m_snd_una;
		}
	}
	if (len > m_t_maxseg) {
		len = m_t_maxseg;
		sendalot = 1;
	}
	if (SEQ_LT(m_snd_nxt + len, m_snd_una + m_send->get_data_at_front ()))
		flags &= ~TH_FIN;

	win = m_recv->get_empty_at_back ();

	/*
	 * Sender silly window avoidance.  If connection is idle
	 * and can send all data, a maximum segment,
	 * at least a maximum default-size segment do it,
	 * or are forced, do it; otherwise don't bother.
	 * If peer's buffer is tiny, then send
	 * when window is at least half open.
	 * If retransmitting (possibly after persist timer forced us
	 * to send into a small window), then must resend.
	 */
	if (len) {
		if (len == m_t_maxseg)
			goto send;
		if ((idle || m_t_flags & TF_NODELAY) &&
		    len + off >= ((int)m_send->get_data_at_front ()))
			goto send;
		if (m_t_force)
			goto send;
		if (len >= ((long)m_max_sndwnd) / 2)
			goto send;
		if (SEQ_LT(m_snd_nxt, m_snd_max))
			goto send;
	}

	/*
	 * Compare available window to amount of window
	 * known to peer (as advertised window less
	 * next expected input).  If the difference is at least two
	 * max size segments, or at least 50% of the maximum possible
	 * window, then want to send a window update to peer.
	 */
	if (win > 0) {
		/* 
		 * "adv" is the amount we can increase the window,
		 * taking into account that we are limited by
		 * TCP_MAXWIN << tp->rcv_scale.
		 */
		long adv = min(win, (long)TCP_MAXWIN << m_rcv_scale) -
			(m_rcv_adv - m_rcv_nxt);

		if (adv >= (long) (2 * m_t_maxseg))
			goto send;
#if 0
		if (2 * adv >= (long) so->so_rcv.sb_hiwat)
			goto send;
#endif
	}

	/*
	 * Send if we owe peer an ACK.
	 */
	if (m_t_flags & TF_ACKNOW)
		goto send;
	if (flags & (TH_SYN|TH_RST))
		goto send;
	if (SEQ_GT(m_snd_up, m_snd_una))
		goto send;
	/*
	 * If our state indicates that FIN should be sent
	 * and we have not yet done so, or we're retransmitting the FIN,
	 * then we need to send.
	 */
	if (flags & TH_FIN &&
	    ((m_t_flags & TF_SENTFIN) == 0 || m_snd_nxt == m_snd_una))
		goto send;

	/*
	 * TCP window updates are not reliable, rather a polling protocol
	 * using ``persist'' packets is used to insure receipt of window
	 * updates.  The three ``states'' for the output side are:
	 *	idle			not doing retransmits or persists
	 *	persisting		to move a small or zero window
	 *	(re)transmitting	and thereby not persisting
	 *
	 * tp->t_timer[TCPT_PERSIST]
	 *	is set when we are in persist state.
	 * tp->t_force
	 *	is set when we are called to send a persist packet.
	 * tp->t_timer[TCPT_REXMT]
	 *	is set when we are retransmitting
	 * The output side is idle when both timers are zero.
	 *
	 * If send window is too small, there is data to transmit, and no
	 * retransmit or persist is pending, then go to persist state.
	 * If nothing happens soon, send when timer expires:
	 * if window is nonzero, transmit what we can,
	 * otherwise force out a byte.
	 */
	if (m_send->get_data_at_front () && m_t_timer[TCPT_REXMT] == 0 &&
	    m_t_timer[TCPT_PERSIST] == 0) {
		m_t_rxtshift = 0;
		setpersist();
		//XXX
	}

	/*
	 * No reason to send a segment, just return.
	 */
	return (0);

 send:
	ChunkTcp *tcp = new ChunkTcp ();
	/*
	 * Before ESTABLISHED, force sending of initial options
	 * unless TCP set not to do any options.
	 * NOTE: we assume that the IP/TCP header plus TCP options
	 * always fit in a single mbuf, leaving room for a maximum
	 * link header, i.e.
	 *	max_linkhdr + sizeof (struct tcpiphdr) + optlen <= MHLEN
	 */
	if (flags & TH_SYN) {
		m_snd_nxt = m_iss;
		if ((m_t_flags & TF_NOOPT) == 0) {
			tcp->enable_option_mss (mss (0));

			if ((m_t_flags & TF_REQ_SCALE) &&
			    ((flags & TH_ACK) == 0 ||
			    (m_t_flags & TF_RCVD_SCALE))) {
				tcp->enable_option_windowscale (m_request_r_scale);
			}
		}
 	}
 
 	/*
	 * Send a timestamp and echo-reply if this is a SYN and our side 
	 * wants to use timestamps (TF_REQ_TSTMP is set) or both our side
	 * and our peer have sent timestamps in our SYN's.
 	 */
 	if ((m_t_flags & (TF_REQ_TSTMP|TF_NOOPT)) == TF_REQ_TSTMP &&
 	     (flags & TH_RST) == 0 &&
 	    ((flags & (TH_SYN|TH_ACK)) == TH_SYN ||
	     (m_t_flags & TF_RCVD_TSTMP))) {
		tcp->enable_option_timestamp (m_host->get_tcp ()->now (), m_ts_recent);
 	}
 
	/*
	 * Adjust data length if insertion of options will
	 * bump the packet length beyond the t_maxseg length.
	 */
	if (len > m_t_maxseg - ((int)tcp->get_size ()) - IPHDRLEN) {
		len = m_t_maxseg - tcp->get_size () - IPHDRLEN;
		sendalot = 1;
	}

	/*
	 * Grab a header mbuf, attaching a copy of data to
	 * be transmitted, and initialize the header from
	 * the template for sends on this connection.
	 */
	Packet *packet;
	if (len) {
		packet = m_send->get_at_front (len);
		/*
		 * If we're sending everything we've got, set PUSH.
		 * (This will keep happy those implementations which only
		 * give data to the user when a buffer fills or
		 * a PUSH comes in.)
		 */
		if (off + len == ((int)m_send->get_data_at_front ()))
			flags |= TH_PUSH;
	} else {
		packet = new Packet ();
	}
	packet->add_header (tcp);

	/*
	 * Fill in fields, remembering maximum advertised
	 * window for use in delaying messages about window sizes.
	 * If resending a FIN, be sure not to use a new sequence number.
	 */
	if (flags & TH_FIN && m_t_flags & TF_SENTFIN && 
	    m_snd_nxt == m_snd_max)
		m_snd_nxt--;
	/*
	 * If we are doing retransmissions, then snd_nxt will
	 * not reflect the first unsent octet.  For ACK only
	 * packets, we do not want the sequence number of the
	 * retransmitted packet, we want the sequence number
	 * of the next unsent octet.  So, if there is no data
	 * (and no SYN or FIN), use snd_max instead of snd_nxt
	 * when filling in ti_seq.  But if we are in persist
	 * state, snd_max might reflect one byte beyond the
	 * right edge of the window, so use snd_nxt in that
	 * case, since we know we aren't doing a retransmission.
	 * (retransmit and persist are mutually exclusive...)
	 */
	if (len || (flags & (TH_SYN|TH_FIN)) || m_t_timer[TCPT_PERSIST])
		tcp->set_sequence_number (m_snd_nxt);
	else
		tcp->set_sequence_number (m_snd_max);
	tcp->set_ack_number (m_rcv_nxt);
	if (flags & TH_SYN) {
		tcp->enable_flag_syn ();
	}
	if (flags & TH_FIN) {
		tcp->enable_flag_fin ();
	}
	if (flags & TH_RST) {
		tcp->enable_flag_rst ();
	}
	if (flags & TH_ACK) {
		tcp->enable_flag_ack ();
	}
	if (flags & TH_PUSH) {
		tcp->enable_flag_psh ();
	}
	/*
	 * Calculate receive window.  Don't shrink window,
	 * but avoid silly window syndrome.
	 */
	if (win < (long)(m_recv->get_size () / 4) && win < (long)m_t_maxseg)
		win = 0;
	if (win > (long)TCP_MAXWIN << m_rcv_scale)
		win = (long)TCP_MAXWIN << m_rcv_scale;
	if (win < (long)(m_rcv_adv - m_rcv_nxt))
		win = (long)(m_rcv_adv - m_rcv_nxt);
	tcp->set_window_size (win>>m_rcv_scale);
	if (SEQ_GT(m_snd_up, m_snd_nxt)) {
		tcp->enable_flag_urg ((u_short)(m_snd_up - m_snd_nxt));
	} else
		/*
		 * If no urgent pointer to send, then we pull
		 * the urgent pointer to the left edge of the send window
		 * so that it doesn't drift into the send window on sequence
		 * number wraparound.
		 */
		m_snd_up = m_snd_una;		/* drag it along */

	/*
	 * In transmit state, time the transmission and arrange for
	 * the retransmit.  In persist state, just set snd_max.
	 */
	if (m_t_force == 0 || m_t_timer[TCPT_PERSIST] == 0) {
		tcp_seq startseq = m_snd_nxt;

		/*
		 * Advance snd_nxt over sequence space of this segment.
		 */
		if (flags & (TH_SYN|TH_FIN)) {
			if (flags & TH_SYN)
				m_snd_nxt++;
			if (flags & TH_FIN) {
				m_snd_nxt++;
				m_t_flags |= TF_SENTFIN;
			}
		}
		m_snd_nxt += len;
		if (SEQ_GT(m_snd_nxt, m_snd_max)) {
			m_snd_max = m_snd_nxt;
			/*
			 * Time this transmission if not a retransmission and
			 * not currently timing anything.
			 */
			if (m_t_rtt == 0) {
				m_t_rtt = 1;
				m_t_rtseq = startseq;
			}
		}

		/*
		 * Set retransmit timer if not currently set,
		 * and not doing an ack or a keep-alive probe.
		 * Initial value for retransmit timer is smoothed
		 * round-trip time + 2 * round-trip time variance.
		 * Initialize shift counter which is used for backoff
		 * of retransmit time.
		 */
		if (m_t_timer[TCPT_REXMT] == 0 &&
		    m_snd_nxt != m_snd_una) {
			m_t_timer[TCPT_REXMT] = m_t_rxtcur;
			if (m_t_timer[TCPT_PERSIST]) {
				m_t_timer[TCPT_PERSIST] = 0;
				m_t_rxtshift = 0;
			}
		}
	} else
		if (SEQ_GT(m_snd_nxt + len, m_snd_max))
			m_snd_max = m_snd_nxt + len;

	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);

	/*
	 * Data sent (as far as we can tell).
	 * If this advertises a larger window than any other segment,
	 * then remember the size of the advertised window.
	 * Any pending ACK has now been sent.
	 */
	if (win > 0 && SEQ_GT(m_rcv_nxt+win, m_rcv_adv))
		m_rcv_adv = m_rcv_nxt + win;
	m_last_ack_sent = m_rcv_nxt;
	m_t_flags &= ~(TF_ACKNOW|TF_DELACK);
	if (sendalot)
		goto again;
	return 0;
}

void
TcpBsdConnection::input (Packet *packet)
{
	caddr_t optp = NULL;
	int tiflags;
	int todrop, acked, ourfinisacked, needoutput = 0;
	int dropsocket = 0;
	int iss = 0;
	u_long tiwin, ts_val, ts_ecr;
	int ts_present = 0;

	ChunkTcp *tcp = static_cast<ChunkTcp *> (packet->remove_header ());
	ChunkPiece *piece = new ChunkPiece ();
	piece->set_original (packet, 0, packet->get_size ());

	tiflags = 0;
	if (tcp->is_flag_syn ()) {
		tiflags |= TH_SYN;
	}
	if (tcp->is_flag_fin ()) {
		tiflags |= TH_FIN;
	}
	if (tcp->is_flag_rst ()) {
		tiflags |= TH_RST;
	}
	if (tcp->is_flag_ack ()) {
		tiflags |= TH_ACK;
	}
	if (tcp->is_flag_psh ()) {
		tiflags |= TH_PUSH;
	}
	
	if (m_t_state == TCPS_CLOSED)
		goto drop;

	/* Unscale the window into a 32-bit value. */
	if ((tiflags & TH_SYN) == 0)
		tiwin = tcp->get_window_size () << m_snd_scale;
	else
		tiwin = tcp->get_window_size ();

	m_t_idle = 0;
	m_t_timer[TCPT_KEEP] = m_tcp_keepidle;

	/*
	 * Process options if not in LISTEN state,
	 * else do it below (after getting remote address).
	 */
	if (optp && m_t_state != TCPS_LISTEN)
		dooptions(tcp, &ts_present, &ts_val, &ts_ecr);


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
	 * Otherwise initialize m_rcv_nxt, and m_irs, select an initial
	 * m_iss, and send a segment:
	 *     <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK>
	 * Also initialize m_snd_nxt to m_iss+1 and m_snd_una to m_iss.
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
		if (optp)
			dooptions(tcp, &ts_present, &ts_val, &ts_ecr);
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
	 *	initialize m_rcv_nxt and m_irs
	 *	if seg contains ack then advance m_snd_una
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
	 *	m_snd_una < ti->ti_ack <= m_snd_max
	 * then advance m_snd_una to ti->ti_ack and drop
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



	
	/*
	 * Process the segment text, merging it into the TCP sequencing queue,
	 * and arranging for acknowledgment of receipt if necessary.
	 * This process logically involves adjusting m_rcv_wnd as data
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

