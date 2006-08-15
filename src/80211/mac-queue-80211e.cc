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

#include "mac-queue-80211e.h"
#include "simulator.h"
#include "packet.h"

using namespace std;

namespace yans {

MacQueue80211e::Item::Item (Packet packet, 
			    ChunkMac80211Hdr const &hdr, 
			    uint64_t tstamp)
	: packet (packet), hdr (hdr), tstamp (tstamp)
{}

MacQueue80211e::MacQueue80211e ()
	: m_size (0)
{}

MacQueue80211e::~MacQueue80211e ()
{
	flush ();
}

void 
MacQueue80211e::set_max_size (uint32_t max_size)
{
	m_max_size = max_size;
}
void 
MacQueue80211e::set_max_delay_us (uint64_t us)
{
	m_max_delay_us = us;
}
void 
MacQueue80211e::enqueue (Packet packet, ChunkMac80211Hdr const &hdr)
{
	cleanup ();
	if (m_size == m_max_size) {
		return;
	}
	uint64_t now = Simulator::now_us ();
	m_queue.push_back (Item (packet, hdr, now));
	m_size++;
}
void
MacQueue80211e::cleanup (void)
{
	if (m_queue.empty ()) {
		return;
	}

	uint64_t now = Simulator::now_us ();
	uint32_t n = 0;
	PacketQueueI end = m_queue.begin ();
	for (PacketQueueI i = m_queue.begin (); i != m_queue.end (); i++) {
		if (i->tstamp + m_max_delay_us > now) {
			end = i;
			break;
		}
		n++;
	}
	m_size -= n;
	m_queue.erase (m_queue.begin (), end);
}

Packet 
MacQueue80211e::dequeue (ChunkMac80211Hdr *hdr, bool *found)
{
	cleanup ();
	if (!m_queue.empty ()) {
		Item i = m_queue.front ();
		m_queue.pop_front ();
		m_size--;
		*hdr = i.hdr;
		*found = true;
		return i.packet;
	}
	*found = false;
	return Packet ();
}


bool
MacQueue80211e::is_empty (void)
{
	cleanup ();
	return m_queue.empty ();
}


uint32_t
MacQueue80211e::get_size (void)
{
	return m_size;
}

void
MacQueue80211e::flush (void)
{
	m_queue.erase (m_queue.begin (), m_queue.end ());
	m_size = 0;
}

}; // namespace yans
