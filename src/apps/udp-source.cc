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

#include "udp-source.h"
#include "event.h"
#include "gpacket.h"
#include "ipv4-end-point.h"
#include "host.h"
#include "udp.h"
#include "tag-ipv4.h"
#include "simulator.h"
#include "event.tcc"

namespace yans {

UdpSource::UdpSource (Host *host)
	: m_host (host),
	  m_end_point (0)
{}
UdpSource::~UdpSource ()
{
	if (m_end_point != 0) {
		delete m_end_point;
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
}


bool 
UdpSource::bind (Ipv4Address address, uint16_t port)
{
	Udp *udp = m_host->get_udp ();
	m_end_point = udp->allocate (address, port);
	m_end_point->set_callback (make_callback (&UdpSource::receive, this));
	if (m_end_point == 0) {
		return false;
	}
	return true;
}
void 
UdpSource::unbind_at (double at)
{
	Simulator::schedule_abs_s (at, make_event (&UdpSource::unbind_now, this));
}
void 
UdpSource::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_address = address;
	m_peer_port = port;
}

void
UdpSource::send (GPacket packet)
{
	if (m_end_point == 0) {
		// not bound.
		return;
	}
	/* route packet. */
	Ipv4Route *routing_table = m_host->get_routing_table ();
	Route *route = routing_table->lookup (m_peer_address);
	if (route == 0) {
		// no route to send packet to. !
		return;
	}
	TagOutPortPair port_tag;
	TagOutIpv4AddressPair addr_tag;
	port_tag.m_sport = m_end_point->get_local_port ();
	port_tag.m_dport = m_peer_port;
	addr_tag.m_daddr = m_peer_address;
	addr_tag.m_saddr = m_end_point->get_local_address ();
	packet.add_tag (&port_tag);
	packet.add_tag (&addr_tag);
	packet.add_tag (route);
	/* send packet. */
	m_host->get_udp ()->send (packet);
}

void 
UdpSource::receive (GPacket packet, Chunk *chunk)
{}

void
UdpSource::unbind_now (void)
{
	delete m_end_point;
	m_end_point = 0;
}


}; // namespace yans
