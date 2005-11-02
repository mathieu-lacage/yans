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

#ifndef UDP_SINK_H
#define UDP_SINK_H

#include <stdint.h>
#include "ipv4-address.h"

class Host;
class Packet;
class Ipv4EndPoint;
class UdpSinkListener;

class UdpSink {
public:
	UdpSink (Host *host);
	~UdpSink ();

	bool bind (Ipv4Address address, uint16_t port);
private:
	friend class UdpSinkListener;
	void receive (Packet *packet);
	Host *m_host;
	Ipv4EndPoint *m_end_point;
	UdpSinkListener *m_listener;
};


#endif /* UDP_SINK_H */
