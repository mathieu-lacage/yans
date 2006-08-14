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

#include "host.h"
#include "arp-ipv4-network-interface.h"
#include "ipv4.h"
#include "ipv4-route.h"
#include "loopback-ipv4.h"
#include "udp.h"
#include "tcp.h"

namespace yans {

Host::Host (char const *path)
{
	m_x = 0.0;
	m_y = 0.0;
	m_z = 0.0;

	m_ipv4 = new Ipv4 ();

	m_udp = new Udp ();
	m_udp->set_host (this);
	m_udp->set_ipv4 (m_ipv4);

	m_tcp = new Tcp ();
	m_tcp->set_host (this);
	m_tcp->set_ipv4 (m_ipv4);

	LoopbackIpv4 *loopback = new LoopbackIpv4 ();	
	uint32_t loopback_id = m_ipv4->add_interface (loopback,
						      Ipv4Address::get_loopback (),
						      Ipv4Mask::get_loopback ());
	m_ipv4->get_routing_table ()->add_host_route_to (Ipv4Address::get_loopback (),
							 loopback_id);
	m_root = new std::string (path);
}

Host::~Host ()
{
	delete m_root;
	delete m_ipv4;
	delete m_udp;
	delete m_tcp;
}

Ipv4Route *
Host::get_routing_table (void)
{
	return m_ipv4->get_routing_table ();
}


uint32_t
Host::add_ipv4_arp_interface (MacNetworkInterface *interface, Ipv4Address address, Ipv4Mask mask)
{
	ArpIpv4NetworkInterface *ipv4 = new ArpIpv4NetworkInterface (interface);
	return m_ipv4->add_interface (ipv4, address, mask);
}

Udp *
Host::get_udp (void)
{
	return m_udp;
}
Tcp *
Host::get_tcp (void)
{
	return m_tcp;
}

double 
Host::get_x (void) const
{
	return m_x;
}
double 
Host::get_y (void) const
{
	return m_y;
}
double 
Host::get_z (void) const
{
	return m_z;
}
void 
Host::set_x (double x)
{
	m_x = x;
}
void 
Host::set_y (double y)
{
	m_y = y;
}
void 
Host::set_z (double z)
{
	m_z = z;
}


}; // namespace yans
