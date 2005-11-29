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


#define TRACE_TCP_CONNECTION 1

#ifdef TRACE_TCP_CONNECTION
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP CONN " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_CONNECTION */
# define TRACE(format,...)
#endif /* TRACE_TCP_CONNECTION */


TcpConnection::TcpConnection ()
{
	m_state = LISTEN;
}
TcpConnection::~TcpConnection ()
{
	delete m_connection_completed;
	delete m_packet_received;
	delete m_ack_received;
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
TcpConnection::set_callbacks (ConnectionCompletedCallback *connection_completed,
			      PacketReceivedCallback *packet_received,
			      AckReceivedCallback *ack_received)
{
	m_connection_completed = connection_completed;
	m_packet_received = packet_received;
	m_ack_received = ack_received;
}


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
	TagOutIpv4 *out_tag = new TagOutIpv4 (m_route);
	out_tag->set_daddress (in_tag->get_saddress ());
	out_tag->set_saddress (in_tag->get_daddress ());
	out_tag->set_dport (in_tag->get_sport ());
	out_tag->set_sport (in_tag->get_dport ());
	packet->add_tag (TagOutIpv4::get_tag (), out_tag);

	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	uint8_t syn;
	if (tcp_chunk->is_flag_syn ()) {
		syn = 1;
	} else {
		syn = 0;
	}
	tcp_chunk->disable_flags ();
	uint16_t old_sp, old_dp;
	uint32_t old_seq, old_payload_size;
	old_sp = tcp_chunk->get_source_port ();
	old_dp = tcp_chunk->get_destination_port ();
	old_seq = tcp_chunk->get_sequence_number ();
	uint32_t old_ack = tcp_chunk->get_ack_number ();
	old_payload_size = packet->get_size () - tcp_chunk->get_size ();
	tcp_chunk->set_source_port (old_dp);
	tcp_chunk->set_destination_port (old_sp);
	tcp_chunk->enable_flag_ack (old_seq + old_payload_size + syn);
	tcp_chunk->set_sequence_number (old_ack);

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
TcpConnection::start_connect (void)
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

	ChunkTcp *tcp_chunk = new ChunkTcp ();		
	tcp_chunk->enable_flag_syn ();
	tcp_chunk->set_source_port (sport);
	tcp_chunk->set_destination_port (dport);
	tcp_chunk->set_sequence_number (get_isn ());
	
	Packet *packet = new Packet ();
	packet->add_tag (TagOutIpv4::get_tag (), out_tag);
	packet->add_header (tcp_chunk);


	TRACE ("send SYN to " << daddress);
	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);

	packet->unref ();
	
	set_state (SYN_SENT);
}

bool
TcpConnection::send_syn_ack (Packet *packet)
{
	if (!invert_packet (packet)) {
		return false;
	}

	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->set_sequence_number (get_isn ());
	tcp_chunk->enable_flag_syn ();

	TRACE ("send SYN+ACK to " << static_cast <TagOutIpv4 *> (packet->get_tag (TagOutIpv4::get_tag ()))->get_daddress ());
	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);

	return true;
}

bool
TcpConnection::send_ack (Packet *packet)
{
	if (!invert_packet (packet)) {
		return false;
	}

	TRACE ("send ACK to " << static_cast <TagOutIpv4 *> (packet->get_tag (TagOutIpv4::get_tag ()))->get_daddress ());
	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);

	return true;
}

void
TcpConnection::send (Packet *packet)
{}


void
TcpConnection::receive (Packet *packet)
{
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
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


