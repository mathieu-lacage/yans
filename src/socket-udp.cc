/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "socket-udp.h"
#include "packet.h"
#include "ipv4-route.h"
#include "host.h"
#include "chunk-data.h"
#include "udp.h"
#include "tag-ipv4.h"

class SocketUdpPacketDestroyNotifier : public PacketDestroyNotifier {
public:
	SocketUdpPacketDestroyNotifier (SocketUdp *socket, uint32_t size)
		: m_socket (socket),
		  m_size (size) {}
	virtual ~SocketUdpPacketDestroyNotifier () {
		m_socket = (SocketUdp *)0xdeadbeaf;
	}
	virtual void notify (Packet *packet) {
		m_socket->notify_packet_destroyed (m_size);
	}
private:
	SocketUdp *m_socket;
	uint32_t m_size;
};

SocketUdp::SocketUdp (Host *host, Udp *udp)
	: m_host (host),
	  m_udp (udp),
	  m_buffer_len (2048),
	  m_used_buffer_len (0)
{}

void 
SocketUdp::close (void)
{
	delete this;
}

void 
SocketUdp::notify_packet_destroyed (uint32_t len)
{
	m_used_buffer_len -= len;
}

void 
SocketUdp::bind (Ipv4Address address, uint16_t port)
{
	m_self_address = address;
	m_self_port = port;
}
Ipv4Address 
SocketUdp::get_ipv4_address (void)
{
	return m_self_address;
}
uint16_t 
SocketUdp::get_port (void)
{
	return m_self_port;
}

void 
SocketUdp::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_address = address;
	m_peer_port = port;
}

uint32_t 
SocketUdp::send (uint8_t const *buf, uint32_t length, int flags)
{
	Ipv4Route *routing_table = m_host->get_routing_table ();
	Route *route = routing_table->lookup (m_peer_address);
	if (route == 0) {
		return 0;
	}
	if (m_used_buffer_len + length > m_buffer_len) {
		return 0;
	} else {
		m_used_buffer_len += length;
	}
	Packet *packet = new Packet ();
	TagOutIpv4 *tag = new TagOutIpv4 (route);
	tag->set_sport (m_self_port);
	tag->set_dport (m_peer_port);
	tag->set_daddress (m_peer_address);
	packet->add_tag (TagOutIpv4::get_tag (), tag);
	packet->add_destroy_notifier (new SocketUdpPacketDestroyNotifier (this, length));
	ChunkData *data = new ChunkData (buf, length);
	packet->add_header (data);
	m_udp->send (packet);
	return length;
}
uint32_t 
SocketUdp::recv (uint8_t const *buf, uint32_t length, int flags)
{
	return 0;
}

