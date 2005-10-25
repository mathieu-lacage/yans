/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "udp.h"
#include "chunk-udp.h"
#include "packet.h"
#include "ipv4.h"
#include "tag-ipv4.h"
#include "ipv4-endpoint.h"
#include "tracer.h"
#include "host.h"

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t Udp::UDP_PROTOCOL = 17;

Udp::Udp ()
{
	m_end_points = new Ipv4EndPoints ();
}

Udp::~Udp ()
{
	delete m_end_points;
}

void 
Udp::set_host (Host *host)
{
	m_host = host;
}
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

Ipv4EndPoints *
Udp::get_end_points (void)
{
	return m_end_points;
}

void 
Udp::receive (Packet *packet)
{
	m_host->get_tracer ()->trace_rx_udp (packet);
	TagInIpv4 *tag = static_cast <TagInIpv4 *> (packet->get_tag (TagInIpv4::get_tag ()));
	assert (tag != 0);
	ChunkUdp *udp_chunk = static_cast <ChunkUdp *> (packet->remove_header ());
	tag->set_dport (udp_chunk->get_destination ());
	tag->set_sport (udp_chunk->get_source ());
	Ipv4EndPoint *end_point = m_end_points->lookup (tag->get_daddress (), tag->get_dport ());
	if (end_point == 0) {
		packet->unref ();
		return;
	}
	end_point->get_listener ()->receive (packet);
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
	m_host->get_tracer ()->trace_tx_udp (packet);
	m_ipv4->send (packet);

	// XXX: if we wanted to, we could generate the udp checksum here.
}
