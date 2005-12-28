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

#ifndef ARP_H
#define ARP_H

#include "mac-address.h"
#include "ipv4-address.h"
#include "sgi-hashmap.h"
#include "callback.tcc"

namespace yans {

class Packet;
class NetworkInterface;
class ArpCacheEntry;

class Arp {
 public:
	typedef Callback<void (Packet *, MacAddress)> ArpSendDataCallback;
	typedef Callback<void (Packet *, MacAddress)> ArpSendArpCallback;
	Arp (NetworkInterface *interface);
	~Arp ();

	void set_alive_timeout (double aliveTimeout);
	void set_dead_timeout (double deadTimeout);
	void set_wait_reply_timeout (double waitReplyTimeout);
	double get_alive_timeout (void);
	double get_dead_timeout (void);
	double get_wait_reply_timeout (void);

	void set_sender (ArpSendDataCallback *send_data,
			 ArpSendArpCallback *send_arp);

	/* send a packet through the ArpMacSender callback. */
	void send_data (Packet *packet, Ipv4Address to);
	/* receive an ARP packet. */
	void recv_arp (Packet *packet);
private:
	typedef Sgi::hash_map<Ipv4Address, ArpCacheEntry *, Ipv4AddressHash> ArpCache;
	typedef Sgi::hash_map<Ipv4Address, ArpCacheEntry *, Ipv4AddressHash>::iterator ArpCacheI;

	void send_arp_reply (Ipv4Address to_ip, MacAddress to_mac);
	void send_arp_request (Ipv4Address to);
	void drop_dead_packet (Packet *packet);

	NetworkInterface *m_interface;
	ArpSendDataCallback *m_send_data;
	ArpSendArpCallback *m_send_arp;
	double m_alive_timeout;
	double m_dead_timeout;
	double m_wait_reply_timeout;
	ArpCache m_arp_cache;
};

}; // namespace yans

#endif /* ARP_H */
