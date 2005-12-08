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

#include "tcp-connection.h"
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


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


#define TRACE_TCP_CONNECTION 1

#ifdef TRACE_TCP_CONNECTION
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP CONN " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_CONNECTION */
# define TRACE(format,...)
#endif /* TRACE_TCP_CONNECTION */

#define TCP_DEFAULT_WINDOW_SIZE (4096)


TcpConnection::TcpConnection ()
{
	m_state = LISTEN;
	m_snd_una = 0;
	m_snd_nxt = 0;
	m_snd_wnd = TCP_DEFAULT_WINDOW_SIZE;
	m_rcv_nxt = 0;
}
TcpConnection::~TcpConnection ()
{
	delete m_connection_completed;
	delete m_data_received;
	delete m_ack_received;
	if (m_destroy != 0) {
		(*m_destroy) (this);
	}
	delete m_destroy;
}

void 
TcpConnection::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
TcpConnection::set_host (Host *host)
{
	m_host = host;
}
void 
TcpConnection::set_end_point (TcpEndPoint *end_point)
{
	m_end_point = end_point;
	m_end_point->set_callback (make_callback (&TcpConnection::receive, this));
}
void 
TcpConnection::set_route (Route *route)
{
	m_route = route;
}
void 
TcpConnection::set_destroy_handler (TcpConnectionDestroy *handler)
{
	m_destroy = handler;
}

void 
TcpConnection::set_callbacks (ConnectionCompletedCallback *connection_completed,
			      DataReceivedCallback *data_received,
			      AckReceivedCallback *ack_received)
{
	m_connection_completed = connection_completed;
	m_data_received = data_received;
	m_ack_received = ack_received;
}

uint32_t 
TcpConnection::get_room_left (void)
{
	return m_send->get_empty_at_back ();
}

uint32_t 
TcpConnection::get_data_ready (void)
{
	return m_recv->get_data_at_front ();
}


uint32_t
TcpConnection::send (Packet *packet)
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
TcpConnection::recv (uint32_t size)
{
	Packet *packet = m_recv->get_at_front (size);
	if (packet == 0) {
		return 0;
	}
	m_recv->remove_at_front (packet->get_size ());
	notify_room_ready_to_receive ();
	return packet;
}

ChunkTcp *
TcpConnection::create_chunk_tcp (void)
{
	ChunkTcp *tcp = new ChunkTcp ();
	tcp->set_source_port (m_end_point->get_local_port ());
	tcp->set_destination_port (m_end_point->get_peer_port ());
	return tcp;
}

void
TcpConnection::invert_packet (Packet *packet)
{
	TagInIpv4 *in_tag = static_cast <TagInIpv4 *> (packet->remove_tag (TagInIpv4::get_tag ()));
	assert (in_tag->get_saddress () == m_end_point->get_peer_address ());
	assert (in_tag->get_sport () == m_end_point->get_peer_port ());
	delete in_tag;

	add_out_tag (packet);
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->disable_flags ();
	tcp_chunk->set_source_port (m_end_point->get_local_port ());
	tcp_chunk->set_destination_port (m_end_point->get_peer_port ());
}

uint32_t
TcpConnection::get_isn (void)
{
	uint32_t isn = Simulator::now_us () & 0xffffffff;
	return isn;
}

void
TcpConnection::add_out_tag (Packet *packet)
{
	uint16_t sport = m_end_point->get_local_port ();
	uint16_t dport = m_end_point->get_peer_port ();
	Ipv4Address saddress = m_end_point->get_local_address ();
	Ipv4Address daddress = m_end_point->get_peer_address ();
	TagOutIpv4 *out_tag = new TagOutIpv4 (m_route);
	out_tag->set_daddress (daddress);
	out_tag->set_saddress (saddress);
	out_tag->set_dport (dport);
	out_tag->set_sport (sport);
	packet->add_tag (TagOutIpv4::get_tag (), out_tag);
}



void
TcpConnection::send_data (void)
{
	uint32_t max_length = min (m_snd_wnd, m_snd_mss);
	uint32_t length = min (m_snd_una+m_snd_wnd-m_snd_nxt, max_length);
	Packet *packet = m_send->get_at (m_snd_nxt, length);
	ChunkTcp *tcp = create_chunk_tcp ();
	tcp->enable_flag_ack ();
	packet->add_header (tcp);
	add_out_tag (packet);
	send_out (packet);
}

void
TcpConnection::notify_data_ready_to_send (void)
{
	if (m_send->get_data_at_front () >= m_snd_mss) {
		/* we can send a full mss. */
		send_data ();
	} else if (m_send->get_data_at_front () >= m_max_sndwnd / 2) {
		/* we can send at least half of the other end's window */
		send_data ();
	} else if (m_snd_una == m_snd_nxt && m_send->get_data_at_front () > 0) {
		/* we are not waiting for an ack. see rfc 896. */
		send_data ();
	}
}

void
TcpConnection::notify_room_ready_to_receive (void)
{}



void 
TcpConnection::set_state (enum TcpState_e new_state)
{
	m_state = new_state;
}


void
TcpConnection::start_connect (void)
{
	Packet *packet = new Packet ();
	add_out_tag (packet);
	ChunkTcp *tcp = create_chunk_tcp ();
	tcp->enable_flag_syn ();
	packet->add_header (tcp);


	TRACE ("send SYN to " << m_end_point->get_peer_address ());
	send_out (packet);
	packet->unref ();	
	set_state (SYN_SENT);
}

void
TcpConnection::start_retransmission_timer (void)
{
	m_retransmission_timer = 10;
}
void
TcpConnection::retransmission_timeout (void)
{
	m_snd_nxt = m_snd_una;
	// XXX retransmit whatever we can.
}

void
TcpConnection::send_out (Packet *packet)
{
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->set_sequence_number (m_snd_nxt);
	tcp_chunk->set_ack_number (m_rcv_nxt);
	tcp_chunk->set_window_size (m_rcv_wnd);

	uint8_t n_seq = 0;
	if (tcp_chunk->is_flag_syn ()) {
		n_seq++;
	}
	if (tcp_chunk->is_flag_fin ()) {
		n_seq++;
	}
	n_seq += packet->get_size () - tcp_chunk->get_size ();
	m_snd_nxt += n_seq;

	start_retransmission_timer ();

	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);
}


void
TcpConnection::send_syn_ack (Packet *packet)
{
	TRACE ("send SYN+ACK to " << static_cast <TagOutIpv4 *> (packet->get_tag (TagOutIpv4::get_tag ()))->get_daddress ());

	invert_packet (packet);
	ChunkTcp *tcp = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp->enable_flag_syn ();
	send_out (packet);
}

void
TcpConnection::send_ack (Packet *packet)
{
	TRACE ("send ACK to " << static_cast <TagOutIpv4 *> (packet->get_tag (TagOutIpv4::get_tag ()))->get_daddress ());
	invert_packet (packet);
	send_out (packet);
}

void
TcpConnection::send_rst (Packet *packet)
{
	invert_packet (packet);
	ChunkTcp *tcp = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp->enable_flag_rst ();
	send_out (packet);
}

void
TcpConnection::cancel_timers (void)
{
	m_retransmission_timer = 0;
	m_2msl_timer = 0;
}

bool 
TcpConnection::is_waiting_ack (void)
{
	if (m_snd_una == m_snd_max) {
		return false;
	} else {
		return true;
	}
}

void
TcpConnection::get_state_flags (bool *send_syn, bool *send_rst, bool *send_ack)
{
	*send_rst = false;
	*send_ack = true;
	*send_syn = false;
	switch (m_state) {
	case CLOSED:
		*send_rst = true;
		break;
	case LISTEN:
		*send_rst = false;
		*send_ack = false;
		break;
	case SYN_SNT:
		*send_syn = true;
		*send_ack = false;
		break;
	case ESTABLISHED:
	case CLOSE_WAIT:
	case FIN_WAIT2:
	case TIME_WAIT:
		break;
	case SYNC_RCVD:
	case FIN_WAIT1:
	case CLOSING:
	case LAST_ACK:
		*send_syn = true;
		break;
	}
}


void
TcpConnection::maybe_segments (void)
{
	bool send_syn, send_rst, send_ack;
	bool sendalot, idle;

	idle = !is_waiting_ack ();
	if (idle && (m_idle_timer > m_rxtcur)) {
		m_snd_cwnd = m_maxseg;
	}

 again:
	sendalot = false;
	get_state_flags (&send_syn, &send_rst, &send_ack);	


	return;

 send:
	if (sendalot) {
		goto again;
	}
	return;
}

void
TcpConnection::receive (Packet *packet)
{
	ChunkTcp *tcp = static_cast <ChunkTcp *> (packet->peek_header ());

	if (m_state == CLOSED) {
		if (!tcp->is_flag_rst ()) {
			send_rst (packet);
		}
		return;
	} else if (m_state == LISTEN) {
		if (tcp->is_flag_rst ()) {
			return;
		} 
		if (tcp->is_flag_ack ()) {
			send_rst (packet);
			return;
		} 
		if (!tcp->is_flag_syn ()) {
			return;
		}
		m_rcv_nxt = tcp->get_sequence_number () + 1;
		m_irs = tcp->get_sequence_number ();
		m_iss = get_isn ();
		m_snd_nxt = m_iss;
		send_syn_ack (packet);
		set_state (SYN_RCVD);
	}  else if (m_state == SYN_SENT) {
		bool ack_ok = false;
		if (tcp->is_flag_ack ()) {
			if (seq_le (tcp->get_ack_number (), m_iss) ||
			    seq_gs (tcp->get_ack_number (), m_snd_nxt)) {
				if (!tcp->is_flag_rst ()) {
					send_rst (packet);
				}
				return;
			}
			if (seq_le (m_snd_una, tcp->get_ack_number ()) &&
			    seq_le (tcp->get_ack_number (), m_snd_nxt)) {
				ack_ok = true;
			}
		}
		if (tcp->is_flag_rst ()) {
			if (ack_ok) {
				// signal "connection reset" error.
				set_state (CLOSED);
			}
			return;
		}
		assert (ack_ok || (!tcp->if_flag_ack () && tcp->is_flag_rst ()));
		if (tcp->is_flag_syn ()) {
			m_rcv_nxt = tcp->get_sequence_number () + 1;
			m_irs = tcp->get_sequence_number ();
			if (tcp->is_flag_ack ()) {
				uint32_t acked = tcp->get_ack_number () - m_snd_una;
				m_snd_una = tcp->get_ack_number ();
				m_send->remove_from_front (acked);
			}
			// XXX: we should deal with data associated to SYN or 
			// SYN+ACK packets.
			if (m_snd_una > m_iss) {
				set_state (ESTABLISHED);
				send_ack (packet);
			} else {
				set_state (SYN_RCVD);
				m_snd_nxt = m_iss;
				send_syn_ack (packet);
			}
			return;
		}
		if (!tcp->is_flag_syn () && !tcp->is_flag_rst ()) {
			return;
		}
	}

 step1:
	uint32_t len = packet->get_size () - tcp->get_size ();
	uint32_t start_trim = 0;
	uint32_t end_trim = 0;
	
	/* trim duplicate data at start of window */
	if (seq_ls (tcp->get_sequence_number (), m_rcv_nxt)) {
		/* remove duplicate syn. */
		if (tcp->is_flag_syn ()) {
			tcp->disable_flag_syn ();
			tcp->set_sequence_number (tcp->get_sequence_number () + 1);
		}
		/* remove duplicate fin. */
		uint32_t fin;
		if (tcp->is_flag_fin ()) {
			fin = 1;
		} else {
			fin = 0;
		}
		if (seq_ls (seq_add (tcp->get_sequence_number (), len + fin),
			    m_rcv_nxt)) {
			tcp->disable_flag_fin ();
			// XXX make sure an ACK is sent.
		}
		start_trim = seq_sub (m_rcv_nxt, tcp->get_sequence_number ());
	}
	/* trim data at end of window. */
	if (seq_gs (seq_add (tcp->get_sequence_number (), len),
		    seq_add (m_rcv_nxt, m_rcv_wnd))) {

		if (seq_ge (tcp->get_sequence_number (),
			    seq_add (m_rcv_nxt, m_rcv_wnd))) {
			/* segment is entirely out of reception window */
			if (m_rcv_wnd == 0 &&
			    tcp->get_sequence_number () == m_rcv_nxt) {
				/* the window is closed. This is probably a window probe. */
				// XXX force ACK to be sent. 
			} else {
				send_ack (packet);
				return;
			}
		}
		uint32_t end_trim = seq_sub (seq_add (tcp->get_sequence_number (), len),
					     seq_add (m_rcv_nxt, m_rcv_wnd));
		tcp->disable_flag_fin ();
	}
	/* really trim segment to window. */
	tcp = static_cast <ChunkTcp *> (packet->remove_header ());
	ChunkPiece *piece = ChunkPiece ();
	piece->set_original (packet, 0, packet->get_size ());
	piece->trim_start (start_trim);
	piece->trim_end (end_trim);

 step2: /* check the rst bit */
	if (tcp->is_flag_rst ()) {
		switch (m_state) {
		case SYN_RCVD:
			// XXX notify connection refused
			set_state (CLOSED);
			break;
		case ESTABLISHED:
		case FIN_WAIT1:
		case FIN_WAIT2:
		case CLOSE_WAIT:
			// XXX notify connection reset
			set_state (CLOSED);
			break;
		case CLOSING:
		case TIME_WAIT:
		case LAST_ACK:
			set_state (CLOSED);
			break;
		}
		delete piece;
		return;
	}
 step4: /* check syn bit */
	if (tcp->is_flag_syn ()) {
		// XXX notify "connection reset"
		set_state (CLOSED);
		delete piece;
		return;
	}
 step5: /* check ack bit */
	if (!tcp->is_flag_ack ()) {
		delete piece;
		return;
	}
	if (m_state == SYN_RCVD) {
		if (seq_le (m_snd_una, tcp->get_ack_number ()) &&
		    seq_le (tcp->get_ack_number (), m_snd_nxt)) {
			m_snd_wl1 = tcp->get_sequence_number () - 1;
			set_state (ESTABLISHED);
			// XXX maybe reass
		} else {
			send_rst (packet);
			delete piece;
			return;
		}
	} else {
		if (seq_le (tcp->get_ack_number (), m_snd_una) &&
		    piece->get_size () == 0 && 
		    tcp->get_window_size () == m_snd_wnd &&
		    m_retransmission_timer > 0 && 
		    tcp->get_ack_number () == m_snd_una) {
			/* this is a duplicate ack */
			m_dupacks ++;
			if (m_dup_acks == m_rexmtthresh) {
				uint32_t win = min (m_snd_wnd, m_snd_cwnd) / 2 / m_maxseg;
				if (win < 2) {
					win = 2;
				}
				m_snd_ssthresh = win * m_maxseg;
				m_retransmission_timer = 0;
				m_rtt = 0;
				m_snd_nxt = seq_min (tcp->get_ack_number (), m_snd_nxt);
				m_snd_cwnd = m_maxseg;
				// XXX tcp_output ?
				m_snd_cwnd = m_snd_ssthresh + m_maxseg * m_dupacks;
				return;
			} else if (m_dup_acks > m_rexmtthresh) {
				m_snd_cwnd += m_mss;
				// XXX tcp_output ?
				return;
			}
		} else {
			m_dupacks = 0;
		}
	}
	if (m_dupacks > m_rexmtthresh &&
	    m_snd_cwnd > m_snd_ssthresh) {
		m_snd_cwnd = m_snd_ssthresh;
	}
	m_dupacks = 0;

	// XXX
	uint32_t acked = seq_sub (tcp->get_ack_number () - m_snd_una);
	// XXX
	

	bool fin_acked;
	if (acked > m_send->get_data_at_front ()) {
		fin_acked = true;
		m_send->remove_at_front (m_send->get_data_at_front ());
		m_snd_wnd -= m_send->get_data_at_front ();
	} else {
		fin_acked = false;
		m_send->remove_at_front (acked);
		m_snd_wnd -= acked;
	}
	(*m_data_transmitted) ();
	
	m_snd_una = tcp->get_ack_number ();
	m_snd_nxt = seq_max (m_snd_nxt, m_snd_una);

	
	switch (m_state) {
	case FIN_WAIT1:
		if (fin_acked) {
			set_state (FIN_WAIT_2);
		}
		break;
	case CLOSING:
		if (fin_acked) {
			// timers ?
			set_state (TIME_WAIT);
		}
		break;
	case LAST_ACK:
		if (fin_acked) {
			set_state (CLOSED);
			return;
		}
		break;
	case TIME_WAIT:
		// finack timer ? XXX
		return;
		break;
	}

	/* update window. */
	if (seq_ls (m_snd_wl1, tcp->get_sequence_number ()) ||
	    (m_snd_wl1 == tcp->get_sequence_number () &&
	     seq_ls (m_snd_wl2, tcp->get_ack_number ())) ||
	    (m_snd_wl2 == tcp->get_ack_number () &&
	     m_snd_wnd < tcp->get_window_size ())) {
		m_snd_wnd = tcp->get_window_size ();
		m_snd_wl1 = tcp->get_sequence_number ();
		m_snd_wl2 = tcp->get_ack_number ();
		m_snd_maxwnd = max (m_snd_wnd, m_snd_maxwnd);
	}

 step6: /* URG bit */
 step7:
	switch (m_state) {
	case ESTABLISHED:
	case FIN_WAIT1:
	case FIN_WAIT2:
		// XXX reass
		
		m_recv->add_at_front (piece);
		break;
	default:
		break;
	}

 step8: /* FIN flag */
	switch (m_state) {
	case CLOSED:
	case LISTEN:
	case SYN_SENT:
		delete piece;
		break;
	default:
		break;
	}
	if (tcp->is_flag_fin ()) {
		// signal "connection closing" XXX
		m_rcv_nxt ++;
		send_ack (packet);
		switch (m_state) {
		case SYN_RCVD:
		case ESTABLISHED:
			set_state (CLOSE_WAIT);
			break;
		case FIN_WAIT1:
			set_state (CLOSING);
			break;
		case FIN_WAIT2:
			cancel_timers ();
			m_2msl_timer = 2 * m_msl;
			set_state (TIME_WAIT);
			break;
		case CLOSE_WAIT:
		case CLOSING:
		case LAST_ACK:
			/* remain in state. */
			break;
		case TIME_WAIT:
			m_2msl_timer = 2 * m_msl;
			break;
		}
	}
}



void 
TcpConnection::slow_timer (void)
{
	if (m_retransmission_timer > 0) {
		m_retransmission_timer--;
		if (m_retransmission_timer == 0) {
			retransmission_timeout ();
		}
	}
}
void 
TcpConnection::fast_timer (void)
{}




bool
TcpConnection::seq_gs (uint32_t a, uint32_t b)
{
	int64_t delta = a - b;
	if (delta < 0) {
		delta = -delta;
	}
	if (delta < (1<<31)) {
		return (a > b);
	} else {
		return (a < b);
	}
}
bool
TcpConnection::seq_le (uint32_t a, uint32_t b)
{
	return !seq_gs (a, b);
}
bool
TcpConnection::seq_ls (uint32_t a, uint32_t b)
{
	return seq_gs (b, a);
}
