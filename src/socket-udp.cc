/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "socket-udp.h"

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
	return 0;
}
uint32_t 
SocketUdp::recv (uint8_t const *buf, uint32_t length, int flags)
{
	return 0;
}

