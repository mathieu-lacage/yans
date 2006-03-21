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
#include "mac-parameters.h"
#include "simulator.h"
#include "packet.h"

using namespace std;

namespace yans {

MacQueue80211e::Item::Item (Packet *packet, 
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
MacQueue80211e::set_parameters (MacParameters *parameters)
{
	m_parameters = parameters;
}

void 
MacQueue80211e::enqueue (Packet *packet, ChunkMac80211Hdr const &hdr)
{
	cleanup ();
	uint64_t now = Simulator::now_us ();
	packet->ref ();
	m_queue.push_front (Item (packet, hdr, now));
	m_size++;
}
void 
MacQueue80211e::enqueue_to_head (Packet *packet, ChunkMac80211Hdr const &hdr)
{
	cleanup ();
	uint64_t now = Simulator::now_us ();
	packet->ref ();
	m_queue.push_back (Item (packet, hdr, now));
	m_size++;
}

void
MacQueue80211e::cleanup (void)
{
	PacketQueueRI tmp;

	if (m_queue.empty ()) {
		return;
	}

	uint64_t now = Simulator::now_us ();
	uint32_t n = 0;
	tmp = m_queue.rbegin ();
	while (tmp != m_queue.rend ()) {
		if (m_size < m_parameters->get_max_queue_size () && 
		    (*tmp).tstamp + m_parameters->get_msdu_lifetime_us () > now) {
			break;
		}
		(*tmp).packet->unref();
		n++;
		tmp++;
		m_size--;
	}
	m_queue.erase (tmp.base (), m_queue.end ());
}

Packet *
MacQueue80211e::dequeue (ChunkMac80211Hdr *hdr)
{
	cleanup ();
	if (!m_queue.empty ()) {
		Item i = m_queue.back ();
		m_queue.pop_back ();
		m_size--;
		*hdr = i.hdr;
		return i.packet;
	}
	return 0;
}

Packet *
MacQueue80211e::look_at_back (ChunkMac80211Hdr *hdr)
{
	cleanup ();
	if (!m_queue.empty ()) {
		Item i = m_queue.back ();
		i.packet->ref ();
		*hdr = i.hdr;
		return i.packet;
	} else {
		return 0;
	}
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
	PacketQueueI tmp;
	for (tmp = m_queue.begin (); tmp != m_queue.end (); tmp++) {
		(*tmp).packet->unref ();
		tmp++;
	}
	m_queue.erase (m_queue.begin (), m_queue.end ());
	m_size = 0;
}

}; // namespace yans
