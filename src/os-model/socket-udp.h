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

#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#include "yans/stdint.h"
#include "ipv4-address.h"

class Udp;
class Host;
class Chunk;

class SocketUdp {
 public:
	void enable_non_blocking (void);
	void enable_blocking (void);

	void close (void);

	/* binding-related operations. */
	void bind (Ipv4Address address, uint16_t port);
	Ipv4Address get_ipv4_address (void);
	uint16_t get_port (void);

	void set_peer (Ipv4Address address, uint16_t port);

	uint32_t send (Chunk *data);
	Chunk *recv (void);
 private:
	friend class SocketUdpPacketDestroyNotifier;
	friend class Host;

	SocketUdp (Host *host, Udp *udp);
	void notify_packet_destroyed (uint32_t len);

	Host *m_host;
	Udp *m_udp;
	Ipv4Address m_self_address;
	uint16_t m_self_port;
	Ipv4Address m_peer_address;
	uint16_t m_peer_port;
	uint32_t m_buffer_len;
	uint32_t m_used_buffer_len;
};

#endif /* SOCKET_UDP */
