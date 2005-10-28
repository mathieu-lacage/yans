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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#include "host.h"
#include "network-interface.h"
#include "ipv4.h"
#include "ipv4-route.h"
#include "loopback-interface.h"
#include "udp.h"
#include "host-tracer.h"

Host::Host (char const *path)
{
	m_ipv4 = new Ipv4 ();
	m_udp = new Udp ();
	m_ipv4->set_host (this);
	m_ipv4->register_transport_protocol (m_udp);
	m_udp->set_host (this);
	m_udp->set_ipv4 (m_ipv4);
	m_routing_table = new Ipv4Route ();
	LoopbackInterface *loopback = new LoopbackInterface ();
	add_interface (loopback);
	loopback->set_ipv4_address (Ipv4Address::get_loopback ());
	loopback->set_ipv4_mask (Ipv4Mask::get_loopback ());
	loopback->set_up ();
	m_routing_table->add_host_route_to (Ipv4Address::get_loopback (),
					    loopback);
	m_root = new std::string (path);
	m_tracer = new HostTracer (0);
}

Host::~Host ()
{
	delete m_root;
	delete m_ipv4;
	delete m_routing_table;
	delete m_udp;
	delete m_tracer;
	for (NetworkInterfacesI i = m_interfaces.begin (); 
	     i != m_interfaces.end (); 
	     i = m_interfaces.erase (i)) {
		delete (*i);
	}
}

Ipv4Route *
Host::get_routing_table (void)
{
	return m_routing_table;
}

NetworkInterfaces const *
Host::get_interfaces (void)
{
	return &m_interfaces;
}

NetworkInterface *
Host::lookup_interface (char const *name)
{
	for (NetworkInterfacesCI i = m_interfaces.begin ();
	     i != m_interfaces.end ();
	     i++) {
		if ((*i)->get_name ()->compare (name) == 0) {
			return (*i);
		}
	}
	return 0;
}
void 
Host::add_interface (NetworkInterface *interface)
{
	m_interfaces.push_back (interface);
	interface->set_ipv4_handler (m_ipv4);
	interface->set_host (this);
}

Udp *
Host::get_udp (void)
{
	return m_udp;
}

HostTracer *
Host::get_tracer (void)
{
	return m_tracer;
}
