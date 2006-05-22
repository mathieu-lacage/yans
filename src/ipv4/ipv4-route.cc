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

#include "ipv4-route.h"
#include "ipv4.h"
#include <cassert>

namespace yans {

/*****************************************************
 *           Network Route
 *****************************************************/

Route::Route ()
{}
Route::Route (Route const &route)
	: m_dest (route.m_dest),
	  m_dest_network_mask (route.m_dest_network_mask),
	  m_gateway (route.m_gateway),
	  m_interface (route.m_interface)
{}

Route::Route (Ipv4Address dest,
	      Ipv4Address gateway,
	      Ipv4NetworkInterface *interface)
	: m_dest (dest),
	  m_dest_network_mask (Ipv4Mask::get_zero ()),
	  m_gateway (gateway),
	  m_interface (interface)
{}
Route::Route (Ipv4Address dest,
	      Ipv4NetworkInterface *interface)
	: m_dest (dest),
	  m_dest_network_mask (Ipv4Mask::get_zero ()),
	  m_gateway (Ipv4Address::get_zero ()),
	  m_interface (interface)
{}
Route::Route (Ipv4Address network,
	      Ipv4Mask network_mask,
	      Ipv4Address gateway,
	      Ipv4NetworkInterface *interface)
	: m_dest (network),
	  m_dest_network_mask (network_mask),
	  m_gateway (gateway),
	  m_interface (interface)
{}
Route::Route (Ipv4Address network,
	      Ipv4Mask network_mask,
	      Ipv4NetworkInterface *interface)
	: m_dest (network),
	  m_dest_network_mask (network_mask),
	  m_gateway (Ipv4Address::get_zero ()),
	  m_interface (interface)
{}

bool 
Route::is_host (void) const
{
	if (m_dest_network_mask.is_equal (Ipv4Mask::get_zero ())) {
		return true;
	} else {
		return false;
	}
}
Ipv4Address 
Route::get_dest (void) const
{
	return m_dest;
}
bool 
Route::is_network (void) const
{
	return !is_host ();
}
bool 
Route::is_default (void) const
{
	if (m_dest.is_equal (Ipv4Address::get_zero ())) {
		return true;
	} else {
		return false;
	}
}
Ipv4Address 
Route::get_dest_network (void) const
{
	return m_dest;
}
Ipv4Mask 
Route::get_dest_network_mask (void) const
{
	return m_dest_network_mask;
}
bool 
Route::is_gateway (void) const
{
	if (m_gateway.is_equal (Ipv4Address::get_zero ())) {
		return false;
	} else {
		return true;
	}
}
Ipv4Address 
Route::get_gateway (void) const
{
	return m_gateway;
}
Ipv4NetworkInterface *
Route::get_interface (void) const
{
	return m_interface;
}

/*****************************************************
 *           Very simple Ipv4 Routing module
 *****************************************************/

Ipv4Route::Ipv4Route ()
	: m_default_route (0)
{}
Ipv4Route::~Ipv4Route ()
{
	for (HostRoutesI i = m_host_routes.begin (); 
	     i != m_host_routes.end (); 
	     i = m_host_routes.erase (i)) {
		delete (*i);
	}
	for (NetworkRoutesI j = m_network_routes.begin (); 
	     j != m_network_routes.end (); 
	     j = m_network_routes.erase (j)) {
		delete (*j);
	}
	delete m_default_route;
	m_default_route = (Route *)0xdeadbeaf;
}


void 
Ipv4Route::add_host_route_to (Ipv4Address dest, 
			      Ipv4Address next_hop, 
			      Ipv4NetworkInterface *interface)
{
	m_host_routes.push_back (new Route (dest, next_hop, interface));
}
void 
Ipv4Route::add_host_route_to (Ipv4Address dest, 
			      Ipv4NetworkInterface *interface)
{
	m_host_routes.push_back (new Route (dest, interface));
}
void 
Ipv4Route::add_network_route_to (Ipv4Address network, 
				 Ipv4Mask network_mask, 
				 Ipv4Address next_hop, 
				 Ipv4NetworkInterface *interface)
{
	m_network_routes.push_back (new Route (network, network_mask, 
					       next_hop, interface));
}
void 
Ipv4Route::add_network_route_to (Ipv4Address network, 
				 Ipv4Mask network_mask, 
				 Ipv4NetworkInterface *interface)
{
	m_network_routes.push_back (new Route (network, network_mask, 
					       interface));
}
void 
Ipv4Route::set_default_route (Ipv4Address next_hop, 
			      Ipv4NetworkInterface *interface)
{
	if (m_default_route != 0) {
		delete m_default_route;
	}
	m_default_route = new Route (Ipv4Address::get_zero (), next_hop, interface);
}
Route *
Ipv4Route::lookup (Ipv4Address dest)
{
	for (HostRoutesCI i = m_host_routes.begin (); 
	     i != m_host_routes.end (); 
	     i++) {
		assert ((*i)->is_host ());
		if ((*i)->get_dest ().is_equal (dest)) {
			return (*i);
		}
	}
	for (NetworkRoutesI j = m_network_routes.begin (); 
	     j != m_network_routes.end (); 
	     j++) {
		assert ((*j)->is_network ());
		Ipv4Mask mask = (*j)->get_dest_network_mask ();
		Ipv4Address entry = (*j)->get_dest_network ();
		if (mask.is_match (dest, entry)) {
			return (*j);
		}
	}
	if (m_default_route != 0) {
		assert (m_default_route->is_default ());
		return m_default_route;
	}
	return 0;
}


}; // namespace yans
