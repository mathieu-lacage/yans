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

#ifndef ARP_CACHE_ENTRY_H
#define ARP_CACHE_ENTRY_H

#include "mac-address.h"
#include "packet.h"

namespace yans {

class ArpIpv4NetworkInterface;

class ArpCacheEntry {
public:
	ArpCacheEntry (ArpIpv4NetworkInterface *arp);

	void mark_dead (void);
	Packet mark_alive (MacAddress mac_address);
	void mark_wait_reply (Packet waiting);
	Packet update_wait_reply (Packet waiting);

	bool is_dead (void);
	bool is_alive (void);
	bool is_wait_reply (void);

	MacAddress get_mac_address (void);
	bool is_expired (void);
private:
	enum ArpCacheEntryState_e {
		ALIVE,
		WAIT_REPLY,
		DEAD
	};

	void update_seen (void);
	ArpCacheEntryState_e get_state (void);
	ArpIpv4NetworkInterface *m_arp;
	ArpCacheEntryState_e m_state;
	uint64_t m_last_seen_time_us;
	MacAddress m_mac_address;
	Packet m_waiting;
};

}; // namespace yans

#endif /* ARP_CACHE_ENTRY_H */
