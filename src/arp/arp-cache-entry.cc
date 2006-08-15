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

#include "simulator.h"
#include "arp-ipv4-network-interface.h"
#include "arp-cache-entry.h"
#include "gpacket.h"
#include <cassert>

namespace yans {

ArpCacheEntry::ArpCacheEntry (ArpIpv4NetworkInterface *arp)
	: m_arp (arp),
	  m_state (ALIVE),
	  m_waiting ()
{}


bool 
ArpCacheEntry::is_dead (void)
{
	return (m_state == DEAD)?true:false;
}
bool 
ArpCacheEntry::is_alive (void)
{
	return (m_state == ALIVE)?true:false;
}
bool
ArpCacheEntry::is_wait_reply (void)
{
	return (m_state == WAIT_REPLY)?true:false;
}


void 
ArpCacheEntry::mark_dead (void) 
{
	m_state = DEAD;
	//assert (m_waiting != 0);
	update_seen ();
}
Packet 
ArpCacheEntry::mark_alive (MacAddress mac_address) 
{
	assert (m_state == WAIT_REPLY);
	//assert (m_waiting != 0);
	m_mac_address = mac_address;
	m_state = ALIVE;
	update_seen ();
	Packet waiting = m_waiting;
	//m_waiting = 0;
	return waiting;
}

Packet 
ArpCacheEntry::update_wait_reply (Packet waiting)
{
	assert (m_state == WAIT_REPLY);
	/* We are already waiting for an answer so
	 * we dump the previously waiting packet and
	 * replace it with this one.
	 */
	Packet old = m_waiting;
	m_waiting = waiting;
	return old;
}
void 
ArpCacheEntry::mark_wait_reply (Packet waiting)
{
	assert (m_state == ALIVE || m_state == DEAD);
	//assert (m_waiting == 0);
	m_state = WAIT_REPLY;
	m_waiting = waiting;
	update_seen ();
}

MacAddress
ArpCacheEntry::get_mac_address (void)
{
	assert (m_state == ALIVE);
	return m_mac_address;
}
bool 
ArpCacheEntry::is_expired (void)
{
	uint64_t timeout_us;
	switch (m_state) {
	case ArpCacheEntry::WAIT_REPLY:
		timeout_us = m_arp->get_wait_reply_timeout_us ();
		break;
	case ArpCacheEntry::DEAD:
		timeout_us = m_arp->get_dead_timeout_us ();
		break;
	case ArpCacheEntry::ALIVE:
		timeout_us = m_arp->get_alive_timeout_us ();
		break;
	default:
		assert (false);
		timeout_us = 0;
		/* NOTREACHED */
		break;
	}
	uint64_t elapsed_since_last_event_us = Simulator::now_us () - m_last_seen_time_us;
	if (elapsed_since_last_event_us >= timeout_us) {
		return true;
	} else {
		return false;
	}
}
void 
ArpCacheEntry::update_seen (void)
{
	m_last_seen_time_us = Simulator::now_us ();
}

}; // namespace yans
