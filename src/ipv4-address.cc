/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "utils.h"
#include "ipv4-address.h"

Ipv4Mask::Ipv4Mask (uint32_t mask)
	: m_mask (mask)
{}
Ipv4Mask::Ipv4Mask (char const *mask)
{
	m_mask = ascii_to_ipv4_host (mask);
}

bool 
Ipv4Mask::is_match (Ipv4Address a, Ipv4Address b)
{
	if ((a.get_host_order () & m_mask) == (b.get_host_order () & m_mask)) {
		return true;
	} else {
		return false;
	}
}

Ipv4Address::Ipv4Address ()
	: m_address (0x66666666)
{}
Ipv4Address::Ipv4Address (uint32_t address)
{
	m_address = address;
}
Ipv4Address::Ipv4Address (char const *address)
{
	m_address = ascii_to_ipv4_host (address);
}

bool 
Ipv4Address::is_equal (Ipv4Address *other)
{
	if (other->m_address == m_address) {
		return true;
	} else {
		return false;
	}
}

uint32_t
Ipv4Address::get_host_order (void)
{
	return m_address;
}
