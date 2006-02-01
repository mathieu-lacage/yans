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

#include "loopback-interface.h"
#include "ipv4.h"
#include <cassert>

namespace yans {

LoopbackInterface::LoopbackInterface ()
	: m_name ("lo"),
	  m_mtu (16436)
{}
LoopbackInterface::~LoopbackInterface ()
{}

void 
LoopbackInterface::set_host (Host *host)
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
void
LoopbackInterface::set_mac_address (MacAddress self)
{
	assert (false);
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
Ipv4Address 
LoopbackInterface::get_ipv4_broadcast (void)
{
	return Ipv4Address::get_broadcast ();
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

}; // namespace yans
