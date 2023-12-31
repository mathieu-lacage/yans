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

#include "yans/packet.h"
#include "yans/packet-logger.h"
#include "yans/trace-container.h"
#include "udp.h"
#include "chunk-udp.h"
#include "ipv4.h"
#include "tag-ipv4.h"
#include "ipv4-end-point.h"
#include "ipv4-end-points.h"
#include <cassert>

namespace yans {

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t Udp::UDP_PROTOCOL = 17;

Udp::Udp ()
{
	m_end_points = new Ipv4EndPoints ();
	m_send_logger = new PacketLogger ();
	m_recv_logger = new PacketLogger ();
}

Udp::~Udp ()
{
	delete m_end_points;
	delete m_send_logger;
	delete m_recv_logger;
}

void 
Udp::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
	m_ipv4->register_transport_protocol (make_callback (&Udp::receive, this), 
					     UDP_PROTOCOL);
}

void
Udp::register_trace (TraceContainer *container)
{
	container->register_packet_logger ("udp-send", m_send_logger);
	container->register_packet_logger ("udp-recv", m_recv_logger);
}

Ipv4EndPoint *
Udp::allocate (void)
{
	return m_end_points->allocate ();
}
Ipv4EndPoint *
Udp::allocate (Ipv4Address address)
{
	return m_end_points->allocate (address);
}
Ipv4EndPoint *
Udp::allocate (Ipv4Address address, uint16_t port)
{
	return m_end_points->allocate (address, port);
}
Ipv4EndPoint *
Udp::allocate (Ipv4Address local_address, uint16_t local_port,
	       Ipv4Address peer_address, uint16_t peer_port)
{
	return m_end_points->allocate (local_address, local_port,
				       peer_address, peer_port);
}



void 
Udp::receive (Packet packet)
{
	m_recv_logger->log (packet);
	ChunkUdp udp_chunk;
	packet.peek (&udp_chunk);
	packet.remove (&udp_chunk);
	TagInPortPair port_tag;
	port_tag.m_dport = udp_chunk.get_destination ();
	port_tag.m_sport = udp_chunk.get_source ();
	packet.add_tag (&port_tag);
	TagInIpv4AddressPair addr_tag;
	packet.peek_tag (&addr_tag);
	Ipv4EndPoint *end_point = m_end_points->lookup (addr_tag.m_daddr, port_tag.m_dport,
							addr_tag.m_saddr, port_tag.m_sport);

	if (end_point == 0) {
		return;
	}
	end_point->receive (packet, &udp_chunk);
}

void
Udp::send (Packet packet)
{
	TagOutPortPair port_tag;
	TagOutIpv4AddressPair addr_tag;
	packet.peek_tag (&port_tag);
	packet.peek_tag (&addr_tag);
	ChunkUdp udp_chunk;
	udp_chunk.set_destination (port_tag.m_dport);
	udp_chunk.set_source (port_tag.m_sport);
	udp_chunk.set_payload_size (packet.get_size ());
	udp_chunk.initialize_checksum (addr_tag.m_saddr,
				       addr_tag.m_daddr,
				       UDP_PROTOCOL);

	packet.add (&udp_chunk);

	m_ipv4->set_protocol (UDP_PROTOCOL);
	m_send_logger->log (packet);
	m_ipv4->send (packet);
}


}; // namespace yans

