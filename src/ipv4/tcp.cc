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

#include "tcp.h"
#include "ipv4-endpoint.h"
#include "ipv4.h"
#include "tag-ipv4.h"
#include "packet.h"
#include "chunk-tcp.h"
#include "host.h"

#define TRACE_TCP 1

#ifdef TRACE_TCP
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP */
# define TRACE(format,...)
#endif /* TRACE_TCP */



Tcp::Tcp ()
{
	m_end_points = new Ipv4EndPoints ();
}
Tcp::~Tcp ()
{}

void 
Tcp::set_host (Host *host)
{
	m_host = host;
}
void 
Tcp::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
	m_ipv4->register_transport_protocol (make_callback (&Tcp::receive, this), 
					     TCP_PROTOCOL);
}

TcpEndPoint *
Tcp::allocate (void)
{
	TcpEndPoint *tcp_end_point = new TcpEndPoint ();
	tcp_end_point->set_ipv4 (m_ipv4);
	Ipv4EndPoint *ipv4_end_point = m_end_points->allocate ();
	tcp_end_point->set_ipv4_end_point (ipv4_end_point);
	tcp_end_point->set_host (m_host);
	return tcp_end_point;
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address)
{
	TcpEndPoint *tcp_end_point = new TcpEndPoint ();
	tcp_end_point->set_ipv4 (m_ipv4);
	Ipv4EndPoint *ipv4_end_point = m_end_points->allocate (address);
	tcp_end_point->set_ipv4_end_point (ipv4_end_point);
	tcp_end_point->set_host (m_host);
	return tcp_end_point;
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address, uint16_t port)
{
	TcpEndPoint *tcp_end_point = new TcpEndPoint ();
	tcp_end_point->set_ipv4 (m_ipv4);
	Ipv4EndPoint *ipv4_end_point = m_end_points->allocate (address, port);
	tcp_end_point->set_ipv4_end_point (ipv4_end_point);
	tcp_end_point->set_host (m_host);
	return tcp_end_point;
}

void
Tcp::send_reset (Packet *packet)
{
	TagInIpv4 *in_tag = static_cast <TagInIpv4 *> (packet->remove_tag (TagInIpv4::get_tag ()));
	Route *route = m_host->get_routing_table ()->lookup (in_tag->get_saddress ());
	if (route == 0) {
		TRACE ("cannot send back RST to " << in_tag->get_saddress ());
		return;
	}
	TagOutIpv4 *out_tag = new TagOutIpv4 (route);
	out_tag->set_daddress (in_tag->get_saddress ());
	out_tag->set_saddress (in_tag->get_daddress ());
	out_tag->set_dport (in_tag->get_sport ());
	out_tag->set_sport (in_tag->get_dport ());
	packet->add_tag (TagOutIpv4::get_tag (), out_tag);

	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->disable_flag_syn ();
	tcp_chunk->enable_flag_rst ();
	uint16_t old_sp, old_dp;
	uint32_t old_seq, old_payload_size;
	old_sp = tcp_chunk->get_source_port ();
	old_dp = tcp_chunk->get_destination_port ();
	old_seq = tcp_chunk->get_sequence_number ();
	old_payload_size = packet->get_size () - tcp_chunk->get_size ();
	tcp_chunk->set_source_port (old_dp);
	tcp_chunk->set_destination_port (old_sp);
	tcp_chunk->enable_flag_ack (old_seq + old_payload_size + 1);
	tcp_chunk->set_sequence_number (0);

	TRACE ("send back RST to " << in_tag->get_saddress ());
	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);
}



void 
Tcp::receive (Packet *packet)
{
	TagInIpv4 *tag = static_cast <TagInIpv4 *> (packet->get_tag (TagInIpv4::get_tag ()));
	assert (tag != 0);
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tag->set_dport (tcp_chunk->get_destination_port ());
	tag->set_sport (tcp_chunk->get_source_port ());
	Ipv4EndPoint *end_point = m_end_points->lookup (tag->get_daddress (), tag->get_dport ());
	if (end_point == 0) {
		if (tcp_chunk->is_flag_syn () &&
		    !tcp_chunk->is_flag_ack ()) {
			/* This is the first SYN packet to open a connection
			 * but no one is expecting this connection to be
			 * opened so we close it with a RST packet.
			 */
			send_reset (packet);
		}
		return;
	}
	(*end_point->peek_callback ()) (packet);
}






TcpEndPoint::TcpEndPoint ()
{
	m_state = LISTEN;
}
TcpEndPoint::~TcpEndPoint ()
{
	delete m_ipv4_end_point;
	delete m_connection_acception;
	delete m_connection_completed;
	delete m_packet_received;
	delete m_ack_received;
}

void 
TcpEndPoint::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
TcpEndPoint::set_ipv4_end_point (Ipv4EndPoint *end_point)
{
	end_point->set_callback (make_callback (&TcpEndPoint::receive, this));
	m_ipv4_end_point = end_point;
}
void 
TcpEndPoint::set_host (Host *host)
{
	m_host = host;
}

void 
TcpEndPoint::set_callbacks (ConnectionAcceptionCallback *connection_acception,
			    ConnectionCompletedCallback *connection_completed,
			    PacketReceivedCallback *packet_received,
			    AckReceivedCallback *ack_received)
{
	m_connection_acception = connection_acception;
	m_connection_completed = connection_completed;
	m_packet_received = packet_received;
	m_ack_received = ack_received;
}


void 
TcpEndPoint::set_state (enum TcpState_e new_state)
{
	m_state = new_state;
}

Route *
TcpEndPoint::lookup_route (Ipv4Address dest)
{
	return m_host->get_routing_table ()->lookup (dest);
}

bool
TcpEndPoint::invert_packet (Packet *packet)
{
	TagInIpv4 *in_tag = static_cast <TagInIpv4 *> (packet->remove_tag (TagInIpv4::get_tag ()));
	Route *route = lookup_route (in_tag->get_saddress ());
	if (route == 0) {
		TRACE ("cannot determine route to " << in_tag->get_saddress ());
		return false;
	}
	TagOutIpv4 *out_tag = new TagOutIpv4 (route);
	out_tag->set_daddress (in_tag->get_saddress ());
	out_tag->set_saddress (in_tag->get_daddress ());
	out_tag->set_dport (in_tag->get_sport ());
	out_tag->set_sport (in_tag->get_dport ());
	packet->add_tag (TagOutIpv4::get_tag (), out_tag);

	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->disable_flags ();
	uint16_t old_sp, old_dp;
	uint32_t old_seq, old_payload_size;
	old_sp = tcp_chunk->get_source_port ();
	old_dp = tcp_chunk->get_destination_port ();
	old_seq = tcp_chunk->get_sequence_number ();
	old_payload_size = packet->get_size () - tcp_chunk->get_size ();
	tcp_chunk->set_source_port (old_dp);
	tcp_chunk->set_destination_port (old_sp);
	tcp_chunk->enable_flag_ack (old_seq + old_payload_size + 1);

	return true;
}

uint32_t
TcpEndPoint::get_isn (void)
{
	uint32_t isn = Simulator::now_us () & 0xffffffff;
	return isn;
}


void
TcpEndPoint::start_connect (Ipv4Address dest, uint16_t port)
{
	m_peer = dest;
	m_peer_port = port;
	m_peer_route = lookup_route (dest);
	if (m_peer_route == 0) {
		return;
	}

	uint16_t sport = m_ipv4_end_point->get_port ();
	uint16_t dport = m_peer_port;
	Ipv4Address daddress = m_peer;
	Ipv4Address saddress = m_ipv4_end_point->get_address ();
	TagOutIpv4 *out_tag = new TagOutIpv4 (m_peer_route);
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
	
	set_state (SYN_SENT);
}

void 
TcpEndPoint::send_syn_ack (Packet *packet)
{
	if (!invert_packet (packet)) {
		return;
	}

	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tcp_chunk->set_sequence_number (get_isn ());

	TRACE ("send SYN+ACK to " << static_cast <TagOutIpv4 *> (packet->remove_tag (TagOutIpv4::get_tag ()))->get_daddress ());
	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);

	set_state (SYN_RCVD);
}

void
TcpEndPoint::send (Packet *packet)
{}


void
TcpEndPoint::receive (Packet *packet)
{
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	switch (m_state) {
	case LISTEN:
		if (tcp_chunk->is_flag_syn () &&
		    !tcp_chunk->is_flag_ack ()) {
			/* this is a connection request. */
			send_syn_ack (packet);
		}
		break;
	case SYN_SENT:
		break;
	case SYN_RCVD:
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


