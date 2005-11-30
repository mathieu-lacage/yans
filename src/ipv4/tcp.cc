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
#include "tcp-connection.h"
#include "tcp-connection-listener.h"
#include "event.h"

#define TRACE_TCP 1

#ifdef TRACE_TCP
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP */
# define TRACE(format,...)
#endif /* TRACE_TCP */

const uint64_t Tcp::FAST_TIMER_DELAY_US = 200000; /* 200ms */
const uint64_t Tcp::SLOW_TIMER_DELAY_US = 500000; /* 500ms */




Tcp::Tcp ()
{
	m_fast_timer = make_static_event (&Tcp::fast_timer, this);
	m_slow_timer = make_static_event (&Tcp::slow_timer, this);
	m_running = false;
}
Tcp::~Tcp ()
{
	delete m_slow_timer;
	delete m_fast_timer;
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

bool
Tcp::lookup_port_local (uint16_t port)
{
	for (TcpEndPointsI i = m_end_p.begin (); i != m_end_p.end (); i++) {
		if ((*i)->get_local_port  () == port) {
			return true;
		}
	}
	return false;
}

bool
Tcp::lookup_local (Ipv4Address addr, uint16_t port)
{
	for (TcpEndPointsI i = m_end_p.begin (); i != m_end_p.end (); i++) {
		if ((*i)->get_local_port () == port &&
		    (*i)->get_local_address () == addr) {
			return true;
		}
	}
	return false;
}

uint16_t
Tcp::allocate_ephemeral_port (void)
{
	uint16_t port = m_ephemeral;
	do {
		port++;
		if (port > 5000) {
			port = 1024;
		}
		if (!lookup_port_local (port)) {
			return port;
			
		}
	} while (port != m_ephemeral);
	return 0;
}

TcpEndPoint *
Tcp::allocate (void)
{
	uint16_t port = allocate_ephemeral_port ();
	if (port == 0) {
		return 0;
	}
	TcpEndPoint *end_point = new TcpEndPoint (Ipv4Address::get_any (), port);
	end_point->set_destroy_callback (make_callback(&Tcp::destroy_end_point, this));
	m_end_p.push_back (end_point);
	return end_point;
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address)
{
	uint16_t port = allocate_ephemeral_port ();
	if (port == 0) {
		return 0;
	}
	TcpEndPoint *end_point = new TcpEndPoint (address, port);
	end_point->set_destroy_callback (make_callback(&Tcp::destroy_end_point, this));
	m_end_p.push_back (end_point);
	return end_point;
}
TcpEndPoint *
Tcp::allocate (Ipv4Address address, uint16_t port)
{
	if (lookup_local (address, port)) {
		return 0;
	}
	TcpEndPoint *end_point = new TcpEndPoint (address, port);
	end_point->set_destroy_callback (make_callback(&Tcp::destroy_end_point, this));
	m_end_p.push_back (end_point);
	return end_point;
}

TcpEndPoint *
Tcp::allocate (Ipv4Address local_address, uint16_t local_port,
	       Ipv4Address peer_address, uint16_t peer_port)
{
	for (TcpEndPointsI i = m_end_p.begin (); i != m_end_p.end (); i++) {
		if ((*i)->get_local_port () == local_port &&
		    (*i)->get_local_address () == local_address &&
		    (*i)->get_peer_port () == peer_port &&
		    (*i)->get_peer_address () == peer_address) {
			/* no way we can allocate this end-point. */
			return 0;
		}
	}
	TcpEndPoint *end_point = new TcpEndPoint (local_address, local_port);
	end_point->set_peer (peer_address, peer_port);
	end_point->set_destroy_callback (make_callback (&Tcp::destroy_end_point, this));
	m_end_p.push_back (end_point);
	return end_point;
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

/*
 * If we have an exact match, we return it.
 * Otherwise, if we find a generic match, we return it.
 * Otherwise, we return 0.
 */
TcpEndPoint *
Tcp::lookup (Ipv4Address daddr, uint16_t dport, Ipv4Address saddr, uint16_t sport)
{
	uint32_t genericity = 3;
	TcpEndPoint *generic = 0;
	//TRACE ("lookup " << daddr << ":" << dport << " " << saddr << ":" << sport);
	for (TcpEndPointsI i = m_end_p.begin (); i != m_end_p.end (); i++) {
#if 0
		TRACE ("against " << 
		       (*i)->get_local_address ()
		       << ":" << 
		       (*i)->get_local_port () 
		       << " " << 
		       (*i)->get_peer_address () 
		       << ":" 
		       << (*i)->get_peer_port ());
#endif
		if ((*i)->get_local_port () != dport) {
			continue;
		}
		if ((*i)->get_local_address () == daddr &&
		    (*i)->get_peer_port () == sport &&
		    (*i)->get_peer_address () == saddr) {
			/* this is an exact match. */
			return *i;
		}
		uint32_t tmp = 0;
		if ((*i)->get_local_address () == Ipv4Address::get_any ()) {
			tmp ++;
		}
		if ((*i)->get_peer_address () == Ipv4Address::get_any ()) {
			tmp ++;
		}
		if (tmp < genericity) {
			generic = (*i);
			genericity = tmp;
		}
	}
	return generic;
}



void
Tcp::receive (Packet *packet)
{
	TagInIpv4 *tag = static_cast <TagInIpv4 *> (packet->get_tag (TagInIpv4::get_tag ()));
	assert (tag != 0);
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	tag->set_dport (tcp_chunk->get_destination_port ());
	tag->set_sport (tcp_chunk->get_source_port ());
	TcpEndPoint *end_p = lookup (tag->get_daddress (), tag->get_dport (), 
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
	TcpConnection *connection = new TcpConnection ();
	connection->set_host (m_host);
	connection->set_ipv4 (m_ipv4);
	connection->set_end_point (end_p);
	Route *route = m_host->get_routing_table ()->lookup (end_p->get_peer_address ());
	connection->set_route (route);
	connection->set_destroy_handler (make_callback (&Tcp::destroy_connection, this));
	if (!m_running) {
		Simulator::insert_in_us (FAST_TIMER_DELAY_US, m_fast_timer);
		Simulator::insert_in_us (SLOW_TIMER_DELAY_US, m_slow_timer);
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
			return;
		}
	}
}

void 
Tcp::destroy_end_point (TcpEndPoint *end_point)
{
	for (TcpEndPointsI i = m_end_p.begin (); i != m_end_p.end (); i++) {
		if ((*i) == end_point) {
			m_end_p.erase (i);
			return;
		}
	}
}
