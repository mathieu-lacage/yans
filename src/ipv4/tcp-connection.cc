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
	m_snd_una = get_isn ();
	m_snd_nxt = m_snd_una;
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

void
TcpConnection::send_data (void)
{
	uint32_t max_length = min (m_snd_wnd, m_snd_mss);
	uint32_t length = min (m_snd_una+m_snd_wnd-m_snd_nxt, max_length);
	Packet *packet = m_send->get_at (m_snd_nxt, length);
	ChunkTcp *tcp = ChunkTcp ();
	tcp->set_source_port (m_end_point->get_local_port ());
	tcp->set_destination_port (m_end_point->get_peer_port ());
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

bool
TcpConnection::invert_packet (Packet *packet)
{
	TagInIpv4 *in_tag = static_cast <TagInIpv4 *> (packet->remove_tag (TagInIpv4::get_tag ()));
	assert (in_tag->get_saddress () == m_end_point->get_peer_address ());
	assert (in_tag->get_sport () == m_end_point->get_peer_port ());
	add_out_tag (packet);

	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->disable_flags ();
	uint16_t old_sp, old_dp;
	old_sp = tcp_chunk->get_source_port ();
	old_dp = tcp_chunk->get_destination_port ();
	tcp_chunk->set_source_port (old_dp);
	tcp_chunk->set_destination_port (old_sp);
	tcp_chunk->enable_flag_ack ();

	delete in_tag;
	return true;
}

uint32_t
TcpConnection::get_isn (void)
{
	uint32_t isn = Simulator::now_us () & 0xffffffff;
	return isn;
}

void
add_out_tag (Packet *packet)
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
TcpConnection::start_connect (void)
{

	uint16_t sport = m_end_point->get_local_port ();
	uint16_t dport = m_end_point->get_peer_port ();

	ChunkTcp *tcp_chunk = new ChunkTcp ();		
	tcp_chunk->enable_flag_syn ();
	tcp_chunk->set_source_port (sport);
	tcp_chunk->set_destination_port (dport);
	
	Packet *packet = new Packet ();
	add_out_tag (packet);
	packet->add_header (tcp_chunk);


	TRACE ("send SYN to " << daddress);
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


bool
TcpConnection::send_syn_ack (Packet *packet)
{
	if (!invert_packet (packet)) {
		return false;
	}

	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->enable_flag_syn ();

	TRACE ("send SYN+ACK to " << static_cast <TagOutIpv4 *> (packet->get_tag (TagOutIpv4::get_tag ()))->get_daddress ());
	send_out (packet);

	return true;
}

bool
TcpConnection::send_ack (Packet *packet)
{
	if (!invert_packet (packet)) {
		return false;
	}

	TRACE ("send ACK to " << static_cast <TagOutIpv4 *> (packet->get_tag (TagOutIpv4::get_tag ()))->get_daddress ());
	send_out (packet);

	return true;
}


void
TcpConnection::receive (Packet *packet)
{
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());


	if (m_state == LISTEN) {
	} else if (m_state == SYN_SENT) {
	}

	switch (m_state) {
	case LISTEN:
		if (tcp_chunk->is_flag_syn () &&
		    !tcp_chunk->is_flag_ack ()) {
			/* this is a connection request. */
			if (send_syn_ack (packet)) {
				set_state (SYN_RCVD);
			}
		}
		break;
	case SYN_SENT:
		if (tcp_chunk->is_flag_syn () &&
		    tcp_chunk->is_flag_ack ()) {
			/* this is the ack of a connection request. */
			if (send_ack (packet)) {
				set_state (ESTABLISHED);
			}
		}
		break;
	case SYN_RCVD:
		if (tcp_chunk->is_flag_ack ()) {
			/* this is the third packet for connection
			 * establishment handshake. 
			 */
			set_state (ESTABLISHED);
		}
		break;
	case ESTABLISHED:
		break;
	case CLOSE_WAIT:
		break;
	case LAST_ACK:
		break;
	case FIN_WAIT_1:
		break;
	case CLOSING:
		break;
	case TIME_WAIT:
		break;
	case FIN_WAIT_2:
		break;
	case CLOSED:
		break;
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
