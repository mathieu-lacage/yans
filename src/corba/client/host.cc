/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "simulation-context.h"
#include "ipv4-network-interface.h"
#include "mac-network-interface.h"
#include "id-factory.h"
#include "ipv4-route.h"

namespace yapns {

Host::Host (SimulationContext context, char const *name)
{
	::Remote::ComputingContext_ptr remote_context = context->peek_remote ();
	m_remote_node = remote_context->create_node (IdFactory::get_next (), name);
	m_routing_table = new Ipv4Route (m_remote_node->get_routing_table ());
}
Host::~Host ()
{
	delete m_routing_table;
	for (Ipv4NetworkInterfacesI i = m_interfaces.begin (); i != m_interfaces.end (); i++) {
		delete (*i);
	}
	m_interfaces.erase (m_interfaces.begin (), m_interfaces.end ());
}

Ipv4Route *
Host::get_routing_table (void)
{
	return m_routing_table;
}

Ipv4NetworkInterface *
Host::add_ipv4_arp_interface (MacNetworkInterface *interface, 
			      Ipv4Address address, Ipv4Mask mask)
{
	::Remote::MacNetworkInterface_ptr remote_mac = interface->get_remote ();
	::Remote::Ipv4NetworkInterface_ptr remote_ipv4 = 
		m_remote_node->add_ipv4_arp_interface (remote_mac,
						       address.get_host_order (),
						       mask.get_host_order (),
						       IdFactory::get_next ());
	Ipv4NetworkInterface *local = new Ipv4NetworkInterface (remote_ipv4);
	CORBA::release (remote_mac);
	CORBA::release (remote_ipv4);
	return local;
}

}; // namespace yapns
