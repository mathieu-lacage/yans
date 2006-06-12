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
#ifndef YAPNS_HOST_H
#define YAPNS_HOST_H

#include <vector>
#include "ipv4-address.h"
#include "simulation-context.h"

namespace yapns {

class Ipv4Route;
class Ipv4NetworkInterface;
class MacNetworkInterface;


class Host {
public:
	Host (SimulationContext context, char const *name);
	~Host ();

	Ipv4Route *get_routing_table (void);

	Ipv4NetworkInterface *add_ipv4_arp_interface (MacNetworkInterface *interface, 
						      Ipv4Address address, Ipv4Mask mask);
 private:
	typedef std::vector<Ipv4NetworkInterface *> Ipv4NetworkInterfaces;
	typedef std::vector<Ipv4NetworkInterface *>::iterator Ipv4NetworkInterfacesI;

	Ipv4Route *m_routing_table;
	::Remote::Node_ptr m_remote_node;
	Ipv4NetworkInterfaces m_interfaces;
};

}; // namespace yapns

#endif /* YAPNS_HOST_H */
