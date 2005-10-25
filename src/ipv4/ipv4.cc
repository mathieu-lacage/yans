/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "ipv4.h"
#include "transport-protocol.h"
#include "chunk-ipv4.h"
#include "packet.h"
#include "network-interface.h"
#include "ipv4-route.h"
#include "host.h"
#include "tag-ipv4.h"

Ipv4::Ipv4 ()
{}
Ipv4::~Ipv4 ()
{}

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
Ipv4::drop_packet (Packet *packet)
{
	packet->unref ();
	// XXX should we print something ?
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

	packet->add_header (ip_header);

	if (packet->get_size () > out_interface->get_mtu ()) {
		/* we need to fragment the packet. */
		// XXX
		assert (false);
	} else {
		if (route->is_gateway ()) {
			out_interface->send (packet, route->get_gateway ());
		} else {
			out_interface->send (packet, tag->get_daddress ());
		}
	}
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
	TagInIpv4 *tag = new TagInIpv4 (interface);
	packet->add_tag (TagInIpv4::get_tag (), tag);
	ChunkIpv4 *ip_header = static_cast <ChunkIpv4 *> (packet->remove_header ());
	/* need to verify if this packet is targetted at _any_ of the
	 * IPs for this host.
	 */
	tag->set_daddress (ip_header->get_destination ());
	TransportProtocol *protocol = lookup_protocol (ip_header->get_protocol ());
	if (protocol == 0) {
		drop_packet (packet);
		return;
	}
	protocol->receive (packet);
}
