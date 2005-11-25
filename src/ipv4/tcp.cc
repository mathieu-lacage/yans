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

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t Tcp::TCP_PROTOCOL = 6;

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
	return tcp_end_point;
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address)
{
	TcpEndPoint *tcp_end_point = new TcpEndPoint ();
	tcp_end_point->set_ipv4 (m_ipv4);
	Ipv4EndPoint *ipv4_end_point = m_end_points->allocate (address);
	tcp_end_point->set_ipv4_end_point (ipv4_end_point);
	return tcp_end_point;
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address, uint16_t port)
{
	TcpEndPoint *tcp_end_point = new TcpEndPoint ();
	tcp_end_point->set_ipv4 (m_ipv4);
	Ipv4EndPoint *ipv4_end_point = m_end_points->allocate (address, port);
	tcp_end_point->set_ipv4_end_point (ipv4_end_point);
	return tcp_end_point;
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
TcpEndPoint::set_peer (Ipv4Address dest, uint16_t port)
{
	m_peer = dest;
	m_peer_port = port;
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


void
TcpEndPoint::start_connect (void)
{
	
	set_state (SYN_SENT);
}

void
TcpEndPoint::send (Packet *packet)
{}


void
TcpEndPoint::receive (Packet *packet)
{
	
}


