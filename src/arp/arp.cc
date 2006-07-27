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

#include "arp.h"
#include "arp-cache-entry.h"
#include "network-interface.h"
#include "packet-logger.h"
#include "trace-container.h"
#include "chunk-arp.h"
#include "packet.h"
#include <cassert>

#define noTRACE_ARP 1

#ifdef TRACE_ARP
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "ARP TRACE " << Simulator::now_us () << "us " \
          << x << std::endl;
#else /* TRACE_ARP */
# define TRACE(format,...)
#endif /* TRACE_ARP */

namespace yans {

Arp::Arp (NetworkInterface *interface)
	: m_interface (interface),
	  m_alive_timeout_us (1200000000),
	  m_dead_timeout_us (100000000),
	  m_wait_reply_timeout_us (1000000),
	  m_drop (new PacketLogger ())
{}
Arp::~Arp ()
{
	flush ();
	delete m_drop;
	m_drop = 0;
}

void 
Arp::flush (void)
{
	for (ArpCacheI i = m_arp_cache.begin (); i != m_arp_cache.end (); i++) {
		delete (*i).second;
	}
	m_arp_cache.erase (m_arp_cache.begin (), m_arp_cache.end ());
}

void 
Arp::register_trace (TraceContainer *container)
{
	container->register_packet_logger ("arp-drop", m_drop);
}

void 
Arp::set_alive_timeout_us (uint64_t alive_timeout_us)
{
	m_alive_timeout_us = alive_timeout_us;
}
void 
Arp::set_dead_timeout_us (uint64_t dead_timeout_us)
{
	m_dead_timeout_us = dead_timeout_us;
}
void 
Arp::set_wait_reply_timeout_us (uint64_t wait_reply_timeout_us)
{
	m_wait_reply_timeout_us = wait_reply_timeout_us;
}

uint64_t
Arp::get_alive_timeout_us (void)
{
	return m_alive_timeout_us;
}
uint64_t
Arp::get_dead_timeout_us (void)
{
	return m_dead_timeout_us;
}
uint64_t
Arp::get_wait_reply_timeout_us (void)
{
	return m_wait_reply_timeout_us;
}


void 
Arp::set_sender (ArpSendDataCallback send_data,
		 ArpSendArpCallback send_arp)
{
	m_send_data = send_data;
	m_send_arp = send_arp;
}

void
Arp::send_arp_request (Ipv4Address to)
{
	ChunkArp arp;
	arp.set_request (m_interface->get_mac_address (),
			 m_interface->get_ipv4_address (),
			 to);
	Packet *packet = new Packet ();
	packet->add (&arp);
	m_send_arp (packet, MacAddress::get_broadcast ());
	packet->unref ();
}

void
Arp::send_arp_reply (Ipv4Address to_ip, MacAddress to_mac)
{
	ChunkArp arp;
	arp.set_reply (m_interface->get_mac_address (),
		       m_interface->get_ipv4_address (),
		       to_mac, to_ip);
	Packet *packet = new Packet ();
	packet->add (&arp);
	m_send_arp (packet, to_mac);
	packet->unref ();
}


void 
Arp::send_data (Packet *packet, Ipv4Address to)
{
	if (m_arp_cache.find (to) != m_arp_cache.end ()) {
		ArpCacheEntry *entry = m_arp_cache[to];
		assert (entry != 0);
		if (entry->is_expired ()) {
			if (entry->is_dead ()) {
				TRACE ("dead entry for " << to << " expired -- send arp request");
				entry->mark_wait_reply (packet);
				send_arp_request (to);
			} else if (entry->is_alive ()) {
				TRACE ("alive entry for " << to << " expired -- send arp request");
				entry->mark_wait_reply (packet);
				send_arp_request (to);
			} else if (entry->is_wait_reply ()) {
				TRACE ("wait reply for " << to << " expired -- drop");
				entry->mark_dead ();
				m_drop->log (packet);
			}
		} else {
			if (entry->is_dead ()) {
				TRACE ("dead entry for " << to << " valid -- drop");
				m_drop->log (packet);
			} else if (entry->is_alive ()) {
				TRACE ("alive entry for " << to << " valid -- send");
				m_send_data (packet, entry->get_mac_address ());
			} else if (entry->is_wait_reply ()) {
				TRACE ("wait reply for " << to << " valid -- drop previous");
				Packet *old = entry->update_wait_reply (packet);
				m_drop->log (old);
				old->unref ();
			}
		}
	} else {
		// This is our first attempt to transmit data to this destination.
		TRACE ("no entry for " << to << " -- send arp request");
		ArpCacheEntry *entry = new ArpCacheEntry (this);
		entry->mark_wait_reply (packet);
		m_arp_cache[to] = entry;
		send_arp_request (to);
	}
}
void 
Arp::recv_arp (Packet *packet)
{
	ChunkArp arp;
	packet->peek (&arp);
	packet->remove (&arp);
	if (arp.is_request () && 
	    arp.get_destination_ipv4_address () == m_interface->get_ipv4_address ()) {
		TRACE ("got request from " << arp.get_source_ipv4_address () << " -- send reply");
		send_arp_reply (arp.get_source_ipv4_address (),
				arp.get_source_hardware_address ());
	} else if (arp.is_reply () &&
		   arp.get_destination_ipv4_address ().is_equal (m_interface->get_ipv4_address ()) &&
		   arp.get_destination_hardware_address ().is_equal (m_interface->get_mac_address ())) {
		Ipv4Address from = arp.get_source_ipv4_address ();
		if (m_arp_cache.find (from) != m_arp_cache.end ()) {
			ArpCacheEntry *entry = m_arp_cache[from];
			assert (entry != 0);
			if (entry->is_wait_reply ()) {
				TRACE ("got reply from " << arp.get_source_ipv4_address ()
				       << " for waiting entry -- flush");
				MacAddress from_mac = arp.get_source_hardware_address ();
				Packet *waiting = entry->mark_alive (from_mac);
				m_send_data (waiting, from_mac);
				waiting->unref ();
			} else {
				// ignore this reply which might well be an attempt 
				// at poisening my arp cache.
				TRACE ("got reply from " << arp.get_source_ipv4_address () << 
				       " for non-waiting entry -- drop");
				m_drop->log (packet);
			}
		} else {
			TRACE ("got reply for unknown entry -- drop");
			m_drop->log (packet);
		}
	}
}

}; // namespace yans
