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

#ifndef UDP_SOURCE_H
#define UDP_SOURCE_H

#include <stdint.h>
#include "ipv4-address.h"
#include "ipv4-endpoint.h"

class Host;
class Ipv4EndPoint;
class UdpSourceListener;

class UdpSource {
public:
	UdpSource (Host *host);
	~UdpSource ();

	/* return true on success. */
	bool bind (Ipv4Address address, uint16_t port);

	void set_peer (Ipv4Address address, uint16_t port);

	void set_packet_interval (double interval);
	void set_packet_size (uint16_t size);

	void start_at (double start);
	void stop_at (double end);

private:
	friend class UdpSourceEvent;
	void send_next_packet (void);

	Host *m_host;
	double m_interval;
	uint16_t m_size;
	double m_stop_at;
	Ipv4EndPoint *m_end_point;
	Ipv4Address m_peer_address;
	uint16_t m_peer_port;
	UdpSourceListener *m_listener;
};

#endif /* UDP_SOURCE_H */
