/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "loopback-interface.h"
#include "ipv4.h"

LoopbackInterface::LoopbackInterface ()
	: m_name ("lo"),
	  m_mtu (16436)
{}
LoopbackInterface::~LoopbackInterface ()
{}

uint16_t 
LoopbackInterface::get_mtu (void)
{
	return m_mtu;
}
std::string const *
LoopbackInterface::get_name (void)
{
	return &m_name;
}
MacAddress 
LoopbackInterface::get_mac_address (void)
{
	return MacAddress::get_broadcast ();
}

void 
LoopbackInterface::set_ipv4_address (Ipv4Address address)
{
	m_address = address;
}
void 
LoopbackInterface::set_ipv4_mask    (Ipv4Mask mask)
{
	m_mask = mask;
}


Ipv4Address
LoopbackInterface::get_ipv4_address (void)
{
	return m_address;
}

Ipv4Mask
LoopbackInterface::get_ipv4_mask (void)
{
	return m_mask;
}


void 
LoopbackInterface::set_ipv4_handler (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}

void 
LoopbackInterface::send (Packet *packet, Ipv4Address to)
{
	m_ipv4->receive (packet, this);
}


void 
LoopbackInterface::set_up   (void)
{
	m_down = false;
}
void 
LoopbackInterface::set_down (void)
{
	m_down = true;
}
bool 
LoopbackInterface::is_down (void)
{
	return m_down;
}

