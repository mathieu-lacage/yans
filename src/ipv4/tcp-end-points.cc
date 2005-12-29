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

#include "tcp-end-points.h"
#include "tcp-end-point.h"

namespace yans {

TcpEndPoints::TcpEndPoints ()
  : m_ephemeral (1025)
{}

TcpEndPoints::~TcpEndPoints ()
{}

bool
TcpEndPoints::lookup_port_local (uint16_t port)
{
	for (EndPointsI i = m_end_points.begin (); i != m_end_points.end (); i++) {
		if ((*i)->get_local_port  () == port) {
			return true;
		}
	}
	return false;
}

bool
TcpEndPoints::lookup_local (Ipv4Address addr, uint16_t port)
{
	for (EndPointsI i = m_end_points.begin (); i != m_end_points.end (); i++) {
		if ((*i)->get_local_port () == port &&
		    (*i)->get_local_address () == addr) {
			return true;
		}
	}
	return false;
}

TcpEndPoint *
TcpEndPoints::allocate (void)
{
	uint16_t port = allocate_ephemeral_port ();
	if (port == 0) {
		return 0;
	}
	TcpEndPoint *end_point = new TcpEndPoint (Ipv4Address::get_any (), port);
	end_point->set_destroy_callback (make_callback_event (&TcpEndPoints::destroy_end_point, this));
	m_end_points.push_back (end_point);
	return end_point;
}
TcpEndPoint *
TcpEndPoints::allocate (Ipv4Address address)
{
	uint16_t port = allocate_ephemeral_port ();
	if (port == 0) {
		return 0;
	}
	TcpEndPoint *end_point = new TcpEndPoint (address, port);
	end_point->set_destroy_callback (make_callback_event (&TcpEndPoints::destroy_end_point, this));
	m_end_points.push_back (end_point);
	return end_point;
}
TcpEndPoint *
TcpEndPoints::allocate (Ipv4Address address, uint16_t port)
{
	if (lookup_local (address, port)) {
		return 0;
	}
	TcpEndPoint *end_point = new TcpEndPoint (address, port);
	end_point->set_destroy_callback (make_callback_event (&TcpEndPoints::destroy_end_point, this));
	m_end_points.push_back (end_point);
	return end_point;
}

TcpEndPoint *
TcpEndPoints::allocate (Ipv4Address local_address, uint16_t local_port,
	       Ipv4Address peer_address, uint16_t peer_port)
{
	for (EndPointsI i = m_end_points.begin (); i != m_end_points.end (); i++) {
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
	end_point->set_destroy_callback (make_callback_event (&TcpEndPoints::destroy_end_point, this));
	m_end_points.push_back (end_point);
	return end_point;
}

/*
 * If we have an exact match, we return it.
 * Otherwise, if we find a generic match, we return it.
 * Otherwise, we return 0.
 */
TcpEndPoint *
TcpEndPoints::lookup (Ipv4Address daddr, uint16_t dport, Ipv4Address saddr, uint16_t sport)
{
	uint32_t genericity = 3;
	TcpEndPoint *generic = 0;
	//TRACE ("lookup " << daddr << ":" << dport << " " << saddr << ":" << sport);
	for (EndPointsI i = m_end_points.begin (); i != m_end_points.end (); i++) {
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
TcpEndPoints::destroy_end_point (TcpEndPoint *end_point)
{
	for (EndPointsI i = m_end_points.begin (); i != m_end_points.end (); i++) {
		if ((*i) == end_point) {
			m_end_points.erase (i);
			return;
		}
	}
}

uint16_t
TcpEndPoints::allocate_ephemeral_port (void)
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

}; // namespace yans
