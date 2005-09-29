/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "loopback-interface.h"

LoopbackInterface::LoopbackInterface ()
	: m_address (Ipv4Address ("127.0.0.1")),
	  m_mask (Ipv4Mask ("255.0.0.0")),
	  m_mtu (16436)
{}
LoopbackInterface::~LoopbackInterface ()
{}

uint16_t 
LoopbackInterface::get_mtu (void)
{
	return m_mtu;
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

MacAddress 
LoopbackInterface::get_mac_address (void)
{
	return MacAddress::get_broadcast ();
}

void 
LoopbackInterface::send_ipv4 (Packet *packet)
{}

