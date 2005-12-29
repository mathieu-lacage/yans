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
#include "packet.h"
#include "chunk-tcp.h"
#include "host.h"
#include "tcp-end-point.h"
#include "tcp-connection-listener.h"
#include "event.tcc"
#include "tcp-end-points.h"

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
	m_fast_timer = make_static_event (&Tcp::fast_timer, this);
	m_slow_timer = make_static_event (&Tcp::slow_timer, this);
	m_running = false;
	m_tcp_now = 0;
	m_tcp_iss = 0;
	m_end_p = new TcpEndPoints ();
}
Tcp::~Tcp ()
{
	delete m_slow_timer;
	delete m_fast_timer;
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

TcpEndPoint *
Tcp::allocate (void)
{
	return m_end_p->allocate ();
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address)
{
	return m_end_p->allocate (address);
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address, uint16_t port)
{
	return m_end_p->allocate (address, port);
}
TcpEndPoint *
Tcp::allocate (Ipv4Address local_address, uint16_t local_port,
	       Ipv4Address peer_address, uint16_t peer_port)
{
	return m_end_p->allocate (local_address, local_port,
				  peer_address, peer_port);
}



void
Tcp::send_reset (Packet *packet)
{
	TagInIpv4 *in_tag = static_cast <TagInIpv4 *> (packet->remove_tag (TagInIpv4::get_tag ()));
	Route *route = m_host->get_routing_table ()->lookup (in_tag->get_saddress ());
	if (route == 0) {
		TRACE ("cannot send back RST to " << in_tag->get_saddress ());
		delete in_tag;
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
	tcp_chunk->set_ack_number (old_seq + old_payload_size + 1);
	tcp_chunk->enable_flag_ack ();
	tcp_chunk->set_sequence_number (0);

	TRACE ("send back RST to " << in_tag->get_saddress ());
	m_ipv4->set_protocol (TCP_PROTOCOL);
	m_ipv4->send (packet);

	delete in_tag;
}



void
Tcp::receive (Packet *packet)
{
	TagInIpv4 *tag = static_cast <TagInIpv4 *> (packet->get_tag (TagInIpv4::get_tag ()));
	assert (tag != 0);
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tag->set_dport (tcp_chunk->get_destination_port ());
	tag->set_sport (tcp_chunk->get_source_port ());
	TcpEndPoint *end_p = m_end_p->lookup (tag->get_daddress (), tag->get_dport (), 
					      tag->get_saddress (), tag->get_sport ());
	if (end_p == 0) {
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
	end_p->receive (packet);
}

TcpConnection *
Tcp::create_connection (TcpEndPoint *end_p)
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
	connection->set_destroy_handler (make_callback_event (&Tcp::destroy_connection, this));
	m_connections.push_back (connection);
	if (!m_running) {
		Simulator::insert_in_us (FAST_TIMER_DELAY_US, m_fast_timer);
		Simulator::insert_in_us (SLOW_TIMER_DELAY_US, m_slow_timer);
		m_running = true;
	}
	return connection;
}
TcpConnectionListener *
Tcp::create_connection_listener (TcpEndPoint *end_p)
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

	Simulator::insert_in_us (SLOW_TIMER_DELAY_US, m_slow_timer);

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

	Simulator::insert_in_us (FAST_TIMER_DELAY_US, m_fast_timer);
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
