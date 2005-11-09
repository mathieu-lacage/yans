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
#include "packet.h"
#include "ipv4-endpoint.h"
#include "host.h"
#include "udp.h"
#include "tag-ipv4.h"
#include "host-tracer.h"


class UdpSourceListener : public Ipv4EndPointListener {
public:
	UdpSourceListener ();
	virtual ~UdpSourceListener ();
	virtual void receive (Packet *packet);
};
UdpSourceListener::UdpSourceListener ()
{}
UdpSourceListener::~UdpSourceListener ()
{}
void UdpSourceListener::receive (Packet *packet)
{
	packet->unref ();
}




UdpSource::UdpSource (Host *host)
	: m_host (host),
	  m_end_point (0),
	  m_listener (new UdpSourceListener ()),
	  m_ref (this)
{}
UdpSource::~UdpSource ()
{
	if (m_end_point != 0) {
		Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
		end_points->destroy (m_end_point);
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
	delete m_listener;
	m_listener = (UdpSourceListener *)0xdeadbeaf;
}

void 
UdpSource::ref (void)
{
	m_ref.ref ();
}
void 
UdpSource::unref (void)
{
	m_ref.unref ();
}

bool 
UdpSource::bind (Ipv4Address address, uint16_t port)
{
	Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
	m_end_point = end_points->allocate (m_listener, address, port);
	if (m_end_point == 0) {
		return false;
	}
	return true;
}
void 
UdpSource::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_address = address;
	m_peer_port = port;
}

void
UdpSource::send (Packet *packet)
{
	/* route packet. */
	Ipv4Route *routing_table = m_host->get_routing_table ();
	Route *route = routing_table->lookup (m_peer_address);
	TagOutIpv4 *tag = new TagOutIpv4 (route);
	tag->set_sport (m_end_point->get_port ());
	tag->set_dport (m_peer_port);
	tag->set_daddress (m_peer_address);
	tag->set_saddress (m_end_point->get_address ());
	packet->add_tag (TagOutIpv4::get_tag (), tag);
	/* send packet. */
	m_host->get_tracer ()->trace_tx_app (packet);
	m_host->get_udp ()->send (packet);
}
