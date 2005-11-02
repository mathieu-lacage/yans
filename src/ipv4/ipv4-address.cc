/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "utils.h"
#include "ipv4-address.h"
#include "buffer.h"

Ipv4Mask Ipv4Mask::m_loopback = Ipv4Mask ("255.0.0.0");
Ipv4Mask Ipv4Mask::m_zero = Ipv4Mask ("0.0.0.0");

Ipv4Mask::Ipv4Mask ()
	: m_mask (0x66666666)
{}

Ipv4Mask::Ipv4Mask (uint32_t mask)
	: m_mask (mask)
{}
Ipv4Mask::Ipv4Mask (char const *mask)
{
	m_mask = ascii_to_ipv4_host (mask);
}

bool 
Ipv4Mask::is_equal (Ipv4Mask other) const
{
	if (other.m_mask == m_mask) {
		return true;
	} else {
		return false;
	}
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

Ipv4Mask
Ipv4Mask::get_loopback (void)
{
	return m_loopback;
}
Ipv4Mask
Ipv4Mask::get_zero (void)
{
	return m_zero;
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
Ipv4Address::is_equal (Ipv4Address other) const
{
	if (other.m_address == m_address) {
		return true;
	} else {
		return false;
	}
}

uint32_t
Ipv4Address::get_host_order (void) const
{
	return m_address;
}

void 
Ipv4Address::serialize (WriteBuffer *buffer)
{
	buffer->write_hton_u32 (m_address);
}
void 
Ipv4Address::deserialize (ReadBuffer *buffer)
{
	m_address = buffer->read_ntoh_u32 ();
}
void 
Ipv4Address::print (std::ostream *os)
{
	*os << ((m_address >> 24) & 0xff) << "."
	    << ((m_address >> 16) & 0xff) << "."
	    << ((m_address >> 8) & 0xff) << "."
	    << ((m_address >> 0) & 0xff);
}


Ipv4Address Ipv4Address::m_zero ("0.0.0.0");
Ipv4Address Ipv4Address::m_broadcast ("255.255.255.255");
Ipv4Address Ipv4Address::m_loopback ("127.0.0.1");

Ipv4Address 
Ipv4Address::get_zero (void)
{
	return m_zero;
}
Ipv4Address 
Ipv4Address::get_any (void)
{
	return m_zero;
}
Ipv4Address 
Ipv4Address::get_broadcast (void)
{
	return m_broadcast;
}
Ipv4Address 
Ipv4Address::get_loopback (void)
{
	return m_broadcast;
}

bool operator == (Ipv4Address const &a, Ipv4Address const &b)
{
	return a.is_equal (b);
}
size_t Ipv4AddressHash::operator()(Ipv4Address const &x) const 
{ 
	return x.get_host_order ();
}
