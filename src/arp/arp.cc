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
#include "chunk-arp.h"
#include "packet.h"

#define noTRACE_ARP 1

#ifdef TRACE_ARP
#include <stdio.h>
#include "simulator.h"
# define TRACE(format,...) \
	printf ("LLARP TRACE %f " format "\n", \
                Simulator::instance ()->now_s (), ## __VA_ARGS__);
#else /* TRACE_ARP */
# define TRACE(format,...)
#endif /* TRACE_ARP */

ArpMacSender::~ArpMacSender ()
{}

Arp::Arp (NetworkInterface *interface)
	: m_interface (interface),
	  m_alive_timeout (1200.0),
	  m_dead_timeout (100.0),
	  m_wait_reply_timeout (1.0)
{}
Arp::~Arp ()
{
	for (ArpCacheI i = m_arp_cache.begin (); i != m_arp_cache.end (); i++) {
		delete (*i).second;
	}
	m_arp_cache.erase (m_arp_cache.begin (), m_arp_cache.end ());
}

void 
Arp::set_alive_timeout (double alive_timeout)
{
	m_alive_timeout = alive_timeout;
}
void 
Arp::set_dead_timeout (double dead_timeout)
{
	m_dead_timeout = dead_timeout;
}
void 
Arp::set_wait_reply_timeout (double wait_reply_timeout)
{
	m_wait_reply_timeout = wait_reply_timeout;
}

double 
Arp::get_alive_timeout (void)
{
	return m_alive_timeout;
}
double 
Arp::get_dead_timeout (void)
{
	return m_dead_timeout;
}
double 
Arp::get_wait_reply_timeout (void)
{
	return m_wait_reply_timeout;
}


void 
Arp::set_sender (ArpMacSender *sender)
{
	m_sender = sender;
}

void
Arp::send_arp_request (Ipv4Address to)
{
	ChunkArp *arp = new ChunkArp ();
	arp->set_request (m_interface->get_mac_address (),
			  m_interface->get_ipv4_address (),
			  to);
	Packet *packet = new Packet ();
	packet->add_header (arp);
	m_sender->send_arp (packet, MacAddress::get_broadcast ());
	packet->unref ();
}

void
Arp::send_arp_reply (Ipv4Address to_ip, MacAddress to_mac)
{
	ChunkArp *arp = new ChunkArp ();
	arp->set_reply (m_interface->get_mac_address (),
			m_interface->get_ipv4_address (),
			to_mac, to_ip);
	Packet *packet = new Packet ();
	packet->add_header (arp);
	m_sender->send_arp (packet, to_mac);
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
				TRACE ("dead entry for %u expired -- send arp request", to.get_host_order ());
				entry->mark_wait_reply (packet);
				send_arp_request (to);
			} else if (entry->is_alive ()) {
				TRACE ("alive entry for %u expired -- send arp request", to.get_host_order ());
				entry->mark_wait_reply (packet);
				send_arp_request (to);
			} else if (entry->is_wait_reply ()) {
				TRACE ("wait reply for %u expired -- drop", to.get_host_order ());
				entry->mark_dead ();
			}
		} else {
			if (entry->is_dead ()) {
				TRACE ("dead entry for %u valid -- drop", to.get_host_order ());
			} else if (entry->is_alive ()) {
				TRACE ("alive entry for %u valid -- send", to.get_host_order ());
				m_sender->send_data (packet, entry->get_mac_address ());
			} else if (entry->is_wait_reply ()) {
				TRACE ("wait reply for %u valid -- drop previous", to.get_host_order ());
				entry->update_wait_reply (packet);
			}
		}
	} else {
		// This is our first attempt to transmit data to this destination.
		TRACE ("no entry for %u -- send arp request", to.get_host_order ());
		ArpCacheEntry *entry = new ArpCacheEntry (this);
		entry->mark_wait_reply (packet);
		m_arp_cache[to] = entry;
		send_arp_request (to);
	}
}
void 
Arp::recv_arp (Packet *packet)
{
	ChunkArp *arp = static_cast <ChunkArp *> (packet->remove_header ());
	if (arp->is_request () && 
	    arp->get_destination_ipv4_address () == m_interface->get_ipv4_address ()) {
		TRACE ("got request from %u -- send reply", arp->get_source_ipv4_address ().get_host_order ());
		send_arp_reply (arp->get_source_ipv4_address (),
				arp->get_source_hardware_address ());
	} else if (arp->is_reply () &&
		   arp->get_destination_ipv4_address ().is_equal (m_interface->get_ipv4_address ()) &&
		   arp->get_destination_hardware_address ().is_equal (m_interface->get_mac_address ())) {
		Ipv4Address from = arp->get_source_ipv4_address ();
		if (m_arp_cache.find (from) != m_arp_cache.end ()) {
			ArpCacheEntry *entry = m_arp_cache[from];
			assert (entry != 0);
			if (entry->is_wait_reply ()) {
				TRACE ("got reply from %u for waiting entry -- flush",
				       arp->get_source_ipv4_address ().get_host_order ());
				MacAddress from_mac = arp->get_source_hardware_address ();
				Packet *waiting = entry->mark_alive (from_mac);
				m_sender->send_data (waiting, from_mac);
				waiting->unref ();
			} else {
				// ignore this reply which might well be an attempt 
				// at poisening my arp cache.
				TRACE ("got reply from %u for non-waiting entry -- drop",
				       arp->get_source_ipv4_address ().get_host_order ());
			}
		} else {
			TRACE ("got reply for unknown entry -- drop");
		}
	}
	delete arp;
}
