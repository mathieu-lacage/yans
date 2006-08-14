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
#include "ipv4.h"
#include "tag-ipv4.h"
#include "gpacket.h"
#include "chunk-tcp.h"
#include "host.h"
#include "ipv4-end-point.h"
#include "tcp-connection-listener.h"
#include "tcp-connection.h"
#include "ipv4-end-points.h"
#include "event.tcc"
#include <cassert>

#ifdef TCP_USE_BSD
#include "tcp-bsd-connection.h"
#endif /* TCP_USE_BSD */

#define TRACE_TCP 1

#ifdef TRACE_TCP
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP */
# define TRACE(format,...)
#endif /* TRACE_TCP */

namespace yans {

const uint64_t Tcp::FAST_TIMER_DELAY_US = 200000; /* 200ms */
const uint64_t Tcp::SLOW_TIMER_DELAY_US = 500000; /* 500ms */




Tcp::Tcp ()
{
	m_running = false;
	m_tcp_now = 0;
	m_tcp_iss = 0;
	m_end_p = new Ipv4EndPoints ();
}
Tcp::~Tcp ()
{
	delete m_end_p;
}

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

Ipv4EndPoint *
Tcp::allocate (void)
{
	return m_end_p->allocate ();
}
Ipv4EndPoint *
Tcp::allocate (Ipv4Address address)
{
	return m_end_p->allocate (address);
}
Ipv4EndPoint *
Tcp::allocate (Ipv4Address address, uint16_t port)
{
	return m_end_p->allocate (address, port);
}
Ipv4EndPoint *
Tcp::allocate (Ipv4Address local_address, uint16_t local_port,
	       Ipv4Address peer_address, uint16_t peer_port)
{
	return m_end_p->allocate (local_address, local_port,
				  peer_address, peer_port);
}



void
Tcp::send_reset (GPacket packet, ChunkTcp *tcp_chunk)
{
	TagInIpv4AddressPair in_addr_tag;
	TagInPortPair in_port_tag;
	packet.peek_tag (&in_addr_tag);
	packet.peek_tag (&in_port_tag);
	Route *route = m_host->get_routing_table ()->lookup (in_addr_tag.m_saddr);
	if (route == 0) {
		TRACE ("cannot send back RST to " << in_addr_tag.m_saddr);
		return;
	}

	GPacket rst;
	
	TagOutIpv4AddressPair out_addr_tag;
	TagOutPortPair out_port_tag;
	out_addr_tag.m_daddr = in_addr_tag.m_saddr;
	out_addr_tag.m_saddr = in_addr_tag.m_daddr;
	out_port_tag.m_dport = in_port_tag.m_sport;
	out_port_tag.m_sport = in_port_tag.m_dport;
	rst.add_tag (&out_addr_tag);
	rst.add_tag (&out_port_tag);
	rst.add_tag (route);

	tcp_chunk->disable_flag_syn ();
	tcp_chunk->enable_flag_rst ();
	uint16_t old_sp, old_dp;
	uint32_t old_seq, old_payload_size;
	old_sp = tcp_chunk->get_source_port ();
	old_dp = tcp_chunk->get_destination_port ();
	old_seq = tcp_chunk->get_sequence_number ();
	old_payload_size = packet.get_size ();
	tcp_chunk->set_source_port (old_dp);
	tcp_chunk->set_destination_port (old_sp);
	tcp_chunk->set_ack_number (old_seq + old_payload_size + 1);
	tcp_chunk->enable_flag_ack ();
	tcp_chunk->set_sequence_number (0);

	rst.add (tcp_chunk);

	TRACE ("send back RST to " << in_addr_tag.m_saddr);
	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (rst);
}



void
Tcp::receive (GPacket packet)
{
	TagInIpv4AddressPair in_addr_tag;
	packet.peek_tag (&in_addr_tag);
	ChunkTcp tcp_chunk;
	packet.peek (&tcp_chunk);
	packet.remove (&tcp_chunk);
	TagInPortPair in_port_tag;
	in_port_tag.m_dport = tcp_chunk.get_destination_port ();
	in_port_tag.m_sport = tcp_chunk.get_source_port ();
	Ipv4EndPoint *end_p = m_end_p->lookup (in_addr_tag.m_daddr, in_port_tag.m_dport, 
					       in_addr_tag.m_saddr, in_port_tag.m_sport);
	packet.add_tag (&in_port_tag);

	if (end_p == 0) {
		if (tcp_chunk.is_flag_syn () &&
		    !tcp_chunk.is_flag_ack ()) {
			/* This is the first SYN packet to open a connection
			 * but no one is expecting this connection to be
			 * opened so we close it with a RST packet.
			 */
			send_reset (packet, &tcp_chunk);
		}
		return;
	}
	end_p->receive (packet, &tcp_chunk);
}

TcpConnection *
Tcp::create_connection (Ipv4EndPoint *end_p)
{
#ifdef TCP_USE_BSD
	TcpBsdConnection *connection = new TcpBsdConnection ();
#else
	TcpConnection *connection = 0;
#endif
	connection->set_host (m_host);
	connection->set_ipv4 (m_ipv4);
	connection->set_end_point (end_p);
	Route *route = m_host->get_routing_table ()->lookup (end_p->get_peer_address ());
	connection->set_route (route);
	connection->set_destroy_handler (make_callback (&Tcp::destroy_connection, this));
	m_connections.push_back (connection);
	if (!m_running) {
		Simulator::schedule_rel_us (FAST_TIMER_DELAY_US, make_event (&Tcp::fast_timer, this));
		Simulator::schedule_rel_us (SLOW_TIMER_DELAY_US, make_event (&Tcp::slow_timer, this));
		m_running = true;
	}
	return connection;
}
TcpConnectionListener *
Tcp::create_connection_listener (Ipv4EndPoint *end_p)
{
	TcpConnectionListener *connection = new TcpConnectionListener ();
	connection->set_host (m_host);
	connection->set_ipv4 (m_ipv4);
	connection->set_end_point (end_p);
	connection->set_tcp (this);
	return connection;
}

int
Tcp::get_new_iss (void)
{
	return m_tcp_iss;
}
int 
Tcp::get_rexmtthresh (void)
{
	return 3;
}

bool
Tcp::is_rfc1323 (void)
{
	return true;
}

int 
Tcp::now (void)
{
	return m_tcp_now;
}


void
Tcp::slow_timer (void)
{
	if (m_connections.empty ()) {
		m_running = false;
		return;
	}

	for (ConnectionsI i = m_connections.begin (); i != m_connections.end (); i++) {
		(*i)->slow_timer ();
	}

	Simulator::schedule_rel_us (SLOW_TIMER_DELAY_US, make_event (&Tcp::slow_timer, this));

	m_tcp_iss += SLOW_TIMER_DELAY_US;
	m_tcp_now++;

}
void
Tcp::fast_timer (void)
{
	if (m_connections.empty ()) {
		m_running = false;
		return;
	}


	for (ConnectionsI i = m_connections.begin (); i != m_connections.end (); i++) {
		(*i)->fast_timer ();
	}

	Simulator::schedule_rel_us (FAST_TIMER_DELAY_US, make_event (&Tcp::fast_timer, this));
}


void 
Tcp::destroy_connection (TcpConnection *connection)
{
	for (ConnectionsI i = m_connections.begin (); i != m_connections.end (); i++) {
		if ((*i) == connection) {
			m_connections.erase (i);
			break;
		}
	}
}

}; // namespace yans
