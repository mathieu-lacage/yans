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

#ifndef HOST_H
#define HOST_H

#include <vector>
#include "ipv4-address.h"

namespace yans {

class Ipv4;
class Ipv4Route;
class Ipv4NetworkInterface;
class SocketUdp;
class Udp;
class Tcp;
class LoopbackIpv4;

typedef std::vector<Ipv4NetworkInterface *> Ipv4NetworkInterfaces;
typedef std::vector<Ipv4NetworkInterface *>::const_iterator Ipv4NetworkInterfacesCI;

class Host {
public:
	Host (char const *path);
	~Host ();

	Ipv4Route *get_routing_table (void);

	Ipv4NetworkInterfaces const *get_interfaces (void);
	void add_interface (Ipv4NetworkInterface *interface);
	
	Udp *get_udp (void);
	Tcp *get_tcp (void);

	double get_x (void) const;
	double get_y (void) const;
	double get_z (void) const;
	void set_x (double x);
	void set_y (double y);
	void set_z (double z);

private:
	typedef std::vector<Ipv4NetworkInterface *>::iterator Ipv4NetworkInterfacesI;

	Ipv4NetworkInterfaces m_interfaces;
	Ipv4Route *m_routing_table;
	Ipv4 *m_ipv4;
	Udp *m_udp;
	Tcp *m_tcp;
	LoopbackIpv4 *m_loopback;
	std::string *m_root;
	double m_x;
	double m_y;
	double m_z;
};

}; // namespace yans


#endif /* HOST_H */
