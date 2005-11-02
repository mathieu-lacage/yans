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

#include "ipv4.h"
#include "transport-protocol.h"
#include "chunk-ipv4.h"
#include "packet.h"
#include "network-interface.h"
#include "ipv4-route.h"
#include "host.h"
#include "tag-ipv4.h"
#include "host-tracer.h"

class IcmpTransportProtocol : public TransportProtocol {
public:
	IcmpTransportProtocol ();
	virtual ~IcmpTransportProtocol ();

	virtual uint8_t get_protocol (void);
	virtual void receive (Packet *packet);
};

IcmpTransportProtocol::IcmpTransportProtocol ()
{}
IcmpTransportProtocol::~IcmpTransportProtocol ()
{}
uint8_t 
IcmpTransportProtocol::get_protocol (void)
{
	return 1;
}
void 
IcmpTransportProtocol::receive (Packet *packet)
{

}



Ipv4::Ipv4 ()
{
	m_icmp = new IcmpTransportProtocol ();
	register_transport_protocol (m_icmp);
}
Ipv4::~Ipv4 ()
{
	delete m_icmp;
}

void 
Ipv4::set_host (Host *host)
{
	m_host = host;
}

Ipv4Route *
Ipv4::get_route (void)
{
	return m_host->get_routing_table ();
}

void 
Ipv4::set_protocol (uint8_t protocol)
{
	m_send_protocol = protocol;
}

void 
Ipv4::send (Packet *packet)
{
	ChunkIpv4 *ip_header;
	ip_header = new ChunkIpv4 ();
	ip_header->set_payload_size (packet->get_size ());

	TagOutIpv4 *tag = static_cast <TagOutIpv4 *> (packet->remove_tag (TagOutIpv4::get_tag ()));
	Route const*route = tag->get_route ();
	assert (route != 0);
	NetworkInterface *out_interface = route->get_interface ();
	ip_header->set_source (tag->get_saddress ());
	ip_header->set_destination (tag->get_daddress ());
	ip_header->set_protocol (m_send_protocol);
	ip_header->set_payload_size (packet->get_size ());

	packet->add_header (ip_header);

	if (packet->get_size () > out_interface->get_mtu ()) {
		/* we need to fragment the packet. */
		// XXX
		assert (false);
	} else {
		m_host->get_tracer ()->trace_tx_ipv4 (packet);
		if (route->is_gateway ()) {
			out_interface->send (packet, route->get_gateway ());
		} else {
			out_interface->send (packet, tag->get_daddress ());
		}
	}
	delete tag;
}

void 
Ipv4::register_transport_protocol (TransportProtocol *protocol)
{
	assert (lookup_protocol (protocol->get_protocol ()) == 0);
	m_protocols.push_back (std::make_pair (protocol->get_protocol (), protocol));
}

TransportProtocol *
Ipv4::lookup_protocol (uint8_t protocol)
{
	for (ProtocolsI i = m_protocols.begin (); i != m_protocols.end (); i++) {
		if ((*i).first == protocol) {
			return (*i).second;
		}
	}
	return 0;
}

void 
Ipv4::receive (Packet *packet, NetworkInterface *interface)
{
	m_host->get_tracer ()->trace_rx_ipv4 (packet);
	ChunkIpv4 *ip_header = static_cast <ChunkIpv4 *> (packet->peek_header ());

	if (!ip_header->is_checksum_ok ()) {
		goto drop_packet;
	}
	NetworkInterfaces const * interfaces = m_host->get_interfaces ();
	for (NetworkInterfacesCI i = interfaces->begin ();
	     i != interfaces->end (); i++) {
		if ((*i)->get_ipv4_address ().is_equal (ip_header->get_destination ())) {
			goto for_us;
		}
	}
	if (ip_header->get_destination ().is_equal (interface->get_ipv4_broadcast ())) {
		goto for_us;
	}
	if (ip_header->get_destination ().is_equal (Ipv4Address::get_broadcast ())) {
		goto for_us;
	}
	if (ip_header->get_destination ().is_equal (Ipv4Address::get_any ())) {
		goto for_us;
	}
	if (ip_header->get_ttl () == 1) {
		//send_icmp_time_exceeded (ip_header->get_source ());
		goto drop_packet;
	}
	ip_header->set_ttl (ip_header->get_ttl () - 1);
	Route *route = m_host->get_routing_table ()->lookup (ip_header->get_destination ());
	if (route->is_gateway ()) {
		route->get_interface ()->send (packet, route->get_gateway ());
	} else {
		route->get_interface ()->send (packet, ip_header->get_destination ());
	}
	return;
 for_us:
	packet->remove_header ();
	TagInIpv4 *tag = new TagInIpv4 (interface);
	packet->add_tag (TagInIpv4::get_tag (), tag);

	tag->set_daddress (ip_header->get_destination ());
	TransportProtocol *protocol = lookup_protocol (ip_header->get_protocol ());
	delete ip_header;
	if (protocol == 0) {
		goto drop_packet;
	}
	protocol->receive (packet);
	return;
 drop_packet:
	return;
}
