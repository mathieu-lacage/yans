/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "ipv4.h"
#include "transport-protocol.h"
#include "chunk-ipv4.h"
#include "packet.h"
#include "network-interface.h"

Ipv4::Ipv4 ()
{}
Ipv4::~Ipv4 ()
{}

void 
Ipv4::set_destination (uint32_t destination)
{
	m_send_destination = destination;
}
void 
Ipv4::set_protocol (uint8_t protocol)
{
	m_send_protocol = protocol;
}
NetworkInterface *
Ipv4::choose_out_interface (uint32_t destination)
{
	return 0;
}
void 
Ipv4::send (Packet *packet)
{
	ChunkIpv4 *ip_header;
	NetworkInterface *interface;
	ip_header = new ChunkIpv4 ();
	ip_header->set_destination (m_send_destination);
	ip_header->set_protocol (m_send_protocol);
	ip_header->set_payload_size (packet->get_size ());

	interface = choose_out_interface (m_send_destination);
	ip_header->set_source (interface->get_ipv4_address ());
	packet->add_header (ip_header);

	if (packet->get_size () > interface->get_mtu ()) {
		/* we need to fragment the packet. */
		// XXX
	} else {
		interface->send_ipv4 (packet);
	}
}

void 
Ipv4::register_network_interface (NetworkInterface *interface)
{
	m_interfaces.push_back (interface);
}
void 
Ipv4::register_transport_protocol (TransportProtocol *protocol)
{
	assert (m_protocols.find (protocol->get_protocol ()) == m_protocols.end ());
	m_protocols[protocol->get_protocol ()] = protocol;
}

void 
Ipv4::receive (Packet *packet)
{}
