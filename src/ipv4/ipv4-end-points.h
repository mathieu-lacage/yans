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

#ifndef TCP_END_POINTS_H
#define TCP_END_POINTS_H

#include <stdint.h>
#include <list>
#include "ipv4-address.h"

namespace yans {

class TcpEndPoint;

class TcpEndPoints {
public:
	TcpEndPoints ();
	~TcpEndPoints ();

	bool lookup_port_local (uint16_t port);
	bool lookup_local (Ipv4Address addr, uint16_t port);
	TcpEndPoint *lookup (Ipv4Address daddr, 
			     uint16_t dport, 
			     Ipv4Address saddr, 
			     uint16_t sport);

	TcpEndPoint *allocate (void);
	TcpEndPoint *allocate (Ipv4Address address);
	TcpEndPoint *allocate (Ipv4Address address, uint16_t port);
	TcpEndPoint *allocate (Ipv4Address local_address, uint16_t local_port,
			       Ipv4Address peer_address, uint16_t peer_port);

 private:
	uint16_t allocate_ephemeral_port (void);
	void destroy_end_point (TcpEndPoint *end_point);
	typedef std::list<TcpEndPoint *> EndPoints;
	typedef std::list<TcpEndPoint *>::iterator EndPointsI;

	uint16_t m_ephemeral;
	EndPoints m_end_points;
};

}; // namespace yans


#endif /* TCP_END_POINTS_H */