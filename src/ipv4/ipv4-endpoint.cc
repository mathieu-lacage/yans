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

#include "ipv4-endpoint.h"

namespace yans {

const uint16_t Ipv4EndPoints::PORT_RESERVED = 5000;



Ipv4EndPoints::Ipv4EndPoints ()
	: m_ephemeral (1024)
{}
Ipv4EndPoints::~Ipv4EndPoints ()
{
	m_ephemeral = 0x6666;
	for (AddressesI i = m_addresses.begin (); i != m_addresses.end (); i++) {
		EndPoints *end_points = (*i).second;
		assert (end_points->empty ());
		end_points->erase (end_points->begin (), end_points->end ());
		delete end_points;
	}
	m_addresses.erase (m_addresses.begin (), m_addresses.end ());
}

Ipv4EndPoint *
Ipv4EndPoints::allocate (void)
{
	Ipv4EndPoint *end_point = new Ipv4EndPoint (this, Ipv4Address::get_any (), 0);
	return end_point;
}
Ipv4EndPoint *
Ipv4EndPoints::allocate (Ipv4Address address)
{
	/* This is the allocation of an ephemeral port. */
	uint16_t port = m_ephemeral;
	do {
		port++;
		if (port > PORT_RESERVED) {
			port = PORT_RESERVED;
		}
		if (lookup (address, port) == 0) {
			Ipv4EndPoint *end_point = insert (address, port);
			return end_point;
		}
	} while (port != m_ephemeral);

	return 0;
}
Ipv4EndPoint *
Ipv4EndPoints::allocate (Ipv4Address address, uint16_t port)
{
	if (lookup (address, port) != 0) {
		return 0;
	}
	Ipv4EndPoint *end_point = insert (address, port);
	return end_point;
}
void
Ipv4EndPoints::destroy (Ipv4EndPoint *end_point)
{
	for (AddressesI i = m_addresses.begin (); i != m_addresses.end (); i++) {
		if ((*i).first.is_equal (end_point->get_address ())) {
			EndPoints *end_points = (*i).second;
			assert (end_points != 0);
			for (EndPointsI j = end_points->begin (); 
			     j != end_points->end (); 
			     j++) {
				if ((*j) == end_point) {
					end_points->erase (j);
					return;
				}
			}
		}
	}
	assert (false);
}

Ipv4EndPoint *
Ipv4EndPoints::insert (Ipv4Address address, uint16_t port)
{
	
	for (AddressesI i = m_addresses.begin (); i != m_addresses.end (); i++) {
		if ((*i).first.is_equal (address)) {
			Ipv4EndPoint *end_point = new Ipv4EndPoint (this, address, port);
			(*i).second->push_back (end_point);
			return end_point;
		}
	}
	m_addresses.push_back (std::make_pair (address, new EndPoints ()));
	Ipv4EndPoint *end_point = insert (address, port);
	return end_point;
}

Ipv4EndPoint *
Ipv4EndPoints::lookup (Ipv4Address address, uint16_t port)
{
	for (AddressesI i = m_addresses.begin (); i != m_addresses.end (); i++) {
		if ((*i).first.is_equal (address)) {
			EndPoints *end_points = (*i).second;
			for (EndPointsI j = end_points->begin (); j != end_points->end (); j++) {
				if ((*j)->get_port () == port) {
					return (*j);
				}
			}
		}
	}
	return 0;
}




uint16_t 
Ipv4EndPoint::get_port (void)
{
	return m_port;
}
Ipv4Address 
Ipv4EndPoint::get_address (void)
{
	return m_address;
}

void
Ipv4EndPoint::set_callback (Ipv4EndPointListener *callback)
{
	m_callback = callback;
}
Ipv4EndPoint::Ipv4EndPointListener *
Ipv4EndPoint::peek_callback (void)
{
	return m_callback;
}
Ipv4EndPoint::Ipv4EndPoint (Ipv4EndPoints *end_points,
			    Ipv4Address address,
			    uint16_t port)
	: m_end_points (end_points),
	  m_address (address),
	  m_port (port)
{}

Ipv4EndPoint::~Ipv4EndPoint ()
{
	m_end_points->destroy (this);
	m_address = Ipv4Address::get_broadcast ();
	m_port = 0x6666;
	m_callback = (Ipv4EndPointListener *)0xdeadbeaf;
}


}; // namespace yans
