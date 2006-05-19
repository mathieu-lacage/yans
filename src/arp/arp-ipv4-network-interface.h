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
#ifndef ARP_IPV4_NETWORK_INTERFACE
#define ARP_IPV4_NETWORK_INTERFACE

#include <stdint.h>
#include "ipv4-network-interface.h"
#include "ipv4-address.h"
#include "mac-address.h"
#include "sgi-hashmap.h"

namespace yans {

class Packet;
class TraceContainer;
class MacNetworkInterface;
class ArpCacheEntry;
class LlcEncapsulation;
class PacketLogger;

class ArpIpv4NetworkInterface : public Ipv4NetworkInterface {
public:
	ArpIpv4NetworkInterface (MacNetworkInterface *interface);
	~ArpIpv4NetworkInterface ();

	void set_alive_timeout_us (uint64_t alive_timeout);
	void set_dead_timeout_us (uint64_t dead_timeout);
	void set_wait_reply_timeout_us (uint64_t wait_reply_timeout);
	uint64_t get_alive_timeout_us (void);
	uint64_t get_dead_timeout_us (void);
	uint64_t get_wait_reply_timeout_us (void);

	void flush (void);

	void register_trace (TraceContainer *container);
private:
	typedef Sgi::hash_map<Ipv4Address, ArpCacheEntry *, Ipv4AddressHash> ArpCache;
	typedef Sgi::hash_map<Ipv4Address, ArpCacheEntry *, Ipv4AddressHash>::iterator ArpCacheI;

	void send_arp_reply (Ipv4Address to_ip, MacAddress to_mac);
	void send_arp_request (Ipv4Address to);
	void drop_dead_packet (Packet *packet);
	void receive_arp (Packet *packet);

	virtual void real_send (Packet *packet, Ipv4Address to);

	MacNetworkInterface *m_interface;
	LlcEncapsulation *m_llc;
	uint64_t m_alive_timeout_us;
	uint64_t m_dead_timeout_us;
	uint64_t m_wait_reply_timeout_us;
	ArpCache m_arp_cache;
	PacketLogger *m_drop;
};


}; // namespace yans

#endif /* ARP_IPV4_NETWORK_INTERFACE */
