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

#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#include "yans/ipv4-address.h"
#include "yans/packet.h"

namespace yans {

class Ipv4;
class Ipv4EndPoints;
class Ipv4EndPoint;
class Host;
class PacketLogger;
class TraceContainer;

class Udp {
public:
	Udp ();
	virtual ~Udp ();

	void set_host (Host *host);
	void set_ipv4 (Ipv4 *ipv4);

	void register_trace (TraceContainer *container);

	Ipv4EndPoint *allocate (void);
	Ipv4EndPoint *allocate (Ipv4Address address);
	Ipv4EndPoint *allocate (Ipv4Address address, uint16_t port);
	Ipv4EndPoint *allocate (Ipv4Address local_address, uint16_t local_port,
			       Ipv4Address peer_address, uint16_t peer_port);

	void send (Packet packet);
 private:
	void receive (Packet packet);

	static const uint8_t UDP_PROTOCOL;
	Ipv4 *m_ipv4;
	Host *m_host;
	Ipv4EndPoints *m_end_points;
	PacketLogger *m_send_logger;
	PacketLogger *m_recv_logger;
};

}; // namespace yans

#endif /* UDP_H */
