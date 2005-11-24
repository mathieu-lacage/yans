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

#ifndef TCP_SOURCE_H
#define TCP_SOURCE_H

#include <stdint.h>
#include "ipv4-address.h"
#include "ipv4-endpoint.h"

class Host;
class Ipv4EndPoint;
class TcpEndPoint;

class TcpSource {
 public:
	TcpSource (Host *host);
	~TcpSource ();

	/* return true on success. */
	bool bind (Ipv4Address address, uint16_t port);
	void set_peer (Ipv4Address address, uint16_t port);
	void start_connect (void);
 private:
	friend class MyTcpTransmissionListener;
	friend class MyTcpReceptionListener;
	friend class MyTcpConnectionListener;
	void send (Packet *packet);
	bool should_accept (Ipv4Address from, uint16_t from_port);
	bool completed (void);
	void receive (Packet *packet);
	void got_ack (Packet *packet);

	Host *m_host;
	Ipv4EndPoint *m_end_point;
	Ipv4Address m_peer_address;
	uint16_t m_peer_port;
	TcpEndPoint *m_tcp_end_point;
};

#endif /* TCP_SOURCE */
