/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "socket-udp.h"

class SocketUdpPacketDestroyNotifier : public PacketDestroyNotifier {
	SocketUdpPacketDestroyNotifier (SocketUdp *socket, uint32_t size)
		: m_socket (socket),
		  m_size (size) {}
	virtual ~SocketUdpPacketDestroyNotifier () {
		m_socket = 0x33;
	}
	virtual void notify (Packet *packet) {
		m_socket->notifyPacketDestroyed (m_size);
	}
private:
	SocketUdp *m_socket;
	uint32_t m_size;
};

SocketUdp::SocketUdp (Host *host)
{}

void 
SocketUdp::close (void)
{
	delete this;
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
	// XXX calculate route.
	if (m_used_buffer_size + length > m_buffer_size) {
		return 0;
	} else {
		m_used_buffer_size += length;
	}
	Packet *packet = new Packet ();
	// associate route to packet.
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

