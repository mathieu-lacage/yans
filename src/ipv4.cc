/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "ipv4.h"
#include "transport-protocol.h"

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
void 
Ipv4::send (Packet *packet)
{
	
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
