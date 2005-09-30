/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "ipv4.h"
#include "transport-protocol.h"
#include "chunk-ipv4.h"
#include "packet.h"
#include "network-interface.h"
#include "ipv4-route.h"
#include "host.h"

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
Ipv4::set_destination (Ipv4Address destination)
{
	m_send_destination = destination;
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
	ip_header->set_destination (m_send_destination);
	ip_header->set_protocol (m_send_protocol);
	ip_header->set_payload_size (packet->get_size ());
	packet->add_header (ip_header);

	NetworkInterface *out_interface;
	HostRoute *host_route = get_route ()->lookup_host (m_send_destination);
	if (host_route == 0 || 
	    host_route->get_interface ()->is_down ()) {
		NetworkRoute *network_route = get_route ()->lookup_network (m_send_destination);
		if (network_route == 0 || 
		    network_route->get_interface ()->is_down ()) {
			DefaultRoute *default_route = get_route ()->lookup_default ();
			if (default_route == 0) {
				// XXX dump packet.
				assert (false);
				return;
			}
			out_interface = default_route->get_interface ();
			Ipv4Address gateway = default_route->get_gateway ();
			out_interface->set_ipv4_next_hop (gateway);
		} else {
			out_interface = network_route->get_interface ();
			if (network_route->is_gateway ()) {
				Ipv4Address gateway = network_route->get_gateway ();
				out_interface->set_ipv4_next_hop (gateway);
			}
		}
	} else {
		out_interface = host_route->get_interface ();
		if (host_route->is_gateway ()) {
			Ipv4Address gateway = host_route->get_gateway ();
			out_interface->set_ipv4_next_hop (gateway);
		}
	}
	ip_header->set_source (out_interface->get_ipv4_address ());

	if (packet->get_size () > out_interface->get_mtu ()) {
		/* we need to fragment the packet. */
		// XXX
		assert (false);
	} else {
		out_interface->send (packet);
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
	 * IP for this host.
	 */
}
