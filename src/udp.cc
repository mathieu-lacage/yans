/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "udp.h"
#include "chunk-udp.h"
#include "packet.h"
#include "ipv4.h"
#include "tag-ipv4.h"

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
Udp::send (Packet *packet)
{
	TagOutIpv4 *tag = static_cast <TagOutIpv4 *> (packet->get_tag (TagOutIpv4::get_tag ()));
	assert (tag != 0);
	ChunkUdp *udp_chunk = new ChunkUdp ();
	udp_chunk->set_destination (tag->get_dport ());
	udp_chunk->set_source (tag->get_sport ());
	udp_chunk->set_payload_size (packet->get_size ());

	packet->add_header (udp_chunk);

	m_ipv4->set_protocol (UDP_PROTOCOL);
	m_ipv4->send (packet);

	// XXX: if we wanted to, we could generate the udp checksum here.
}
