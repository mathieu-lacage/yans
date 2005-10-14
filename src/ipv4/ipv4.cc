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
	ip_header->set_source (out_interface->get_ipv4_address ());
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
	assert (m_protocols.find (protocol->get_protocol ()) == m_protocols.end ());
	m_protocols[protocol->get_protocol ()] = protocol;
}

void 
Ipv4::receive (Packet *packet, NetworkInterface *interface)
{
	//ChunkIpv4 *ip_header = static_cast <ChunkIpv4 *> (packet->remove_header ());
	/* need to verify if this packet is targetted at _any_ of the
	 * IPs for this host.
	 */
}
