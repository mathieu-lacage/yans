/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "mac-network-interface.h"

namespace yans {

MacNetworkInterface::MacNetworkInterface (MacAddress self, uint16_t mtu)
	: m_self (self), m_mtu (mtu)
{}
MacNetworkInterface::~MacNetworkInterface ()
{}

MacAddress 
MacNetworkInterface::get_mac_address (void) const
{
	return m_self;
}
uint16_t 
MacNetworkInterface::get_mtu (void) const
{
	return m_mtu;
}
bool 
MacNetworkInterface::is_down (void) const
{
	return m_is_down;
}
void 
MacNetworkInterface::set_up   (void)
{
	m_is_down = false;
	notify_up ();
}
void 
MacNetworkInterface::set_down (void)
{
	m_is_down = true;
	notify_down ();
}

void 
MacNetworkInterface::set_rx_callback (RxCallback callback)
{
	m_rx_callback = callback;
}
void 
MacNetworkInterface::send (Packet *packet, MacAddress to)
{
	real_send (packet, to);
}
void 
MacNetworkInterface::forward_up (Packet *packet)
{
	m_rx_callback (packet, this);
}

}; // namespace yans