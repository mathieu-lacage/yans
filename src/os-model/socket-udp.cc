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

#include "socket-udp.h"
#include "packet.h"
#include "ipv4-route.h"
#include "host.h"
#include "chunk-data.h"
#include "udp.h"
#include "tag-ipv4.h"

class SocketUdpPacketDestroyNotifier : public PacketDestroyNotifier {
public:
	SocketUdpPacketDestroyNotifier (SocketUdp *socket, uint32_t size)
		: m_socket (socket),
		  m_size (size) {}
	virtual ~SocketUdpPacketDestroyNotifier () {
		m_socket = (SocketUdp *)0xdeadbeaf;
	}
	virtual void notify (Packet *packet) {
		m_socket->notify_packet_destroyed (m_size);
	}
private:
	SocketUdp *m_socket;
	uint32_t m_size;
};

SocketUdp::SocketUdp (Host *host, Udp *udp)
	: m_host (host),
	  m_udp (udp),
	  m_buffer_len (2048),
	  m_used_buffer_len (0)
{}

void 
SocketUdp::close (void)
{
	delete this;
}

void 
SocketUdp::notify_packet_destroyed (uint32_t len)
{
	m_used_buffer_len -= len;
}

void 
SocketUdp::bind (Ipv4Address address, uint16_t port)
{
	m_self_address = address;
	m_self_port = port;
}
Ipv4Address 
SocketUdp::get_ipv4_address (void)
{
	return m_self_address;
}
uint16_t 
SocketUdp::get_port (void)
{
	return m_self_port;
}

void 
SocketUdp::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_address = address;
	m_peer_port = port;
}

uint32_t
SocketUdp::send (Chunk *data)
{
	Ipv4Route *routing_table = m_host->get_routing_table ();
	Route *route = routing_table->lookup (m_peer_address);
	if (route == 0) {
		return 0;
	}
	if (m_used_buffer_len + data->get_size () > m_buffer_len) {
		return 0;
	} else {
		m_used_buffer_len += data->get_size ();
	}
	Packet *packet = new Packet ();
	TagOutIpv4 *tag = new TagOutIpv4 (route);
	tag->set_sport (m_self_port);
	tag->set_dport (m_peer_port);
	tag->set_daddress (m_peer_address);
	packet->add_tag (TagOutIpv4::get_tag (), tag);
	packet->add_destroy_notifier (new SocketUdpPacketDestroyNotifier (this, data->get_size ()));
	packet->add_header (data);
	m_udp->send (packet);
	return data->get_size ();
}
Chunk *
SocketUdp::recv (void)
{
	return 0;
}

