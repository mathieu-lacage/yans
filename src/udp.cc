/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "udp.h"
#include "chunk-udp.h"
#include "packet.h"
#include "ipv4.h"

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t Udp::UDP_PROTOCOL = 17;

Udp::~Udp ()
{}

void 
Udp::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}

uint8_t 
Udp::get_protocol (void)
{
	return UDP_PROTOCOL;
}

void 
Udp::receive (Packet *packet)
{
}

void 
Udp::set_destination (Ipv4Address dest)
{
	m_destination = dest;
}
void 
Udp::set_destination (uint16_t port)
{
	m_destination_port = port;
}
void 
Udp::set_source (uint16_t port)
{
	m_source_port = port;
}
		


void
Udp::send (Packet *packet)
{
	ChunkUdp *udp_chunk = new ChunkUdp ();
	udp_chunk->set_destination (m_destination_port);
	udp_chunk->set_source (m_source_port);
	packet->add_header (udp_chunk);
	m_ipv4->set_protocol (UDP_PROTOCOL);
	m_ipv4->set_destination (m_destination);
	m_ipv4->send (packet);
}
