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
#include "packet.h"

#include <iostream>

using namespace std;

MacQueue80211e::MacQueue80211e (MacParameters *parameters)
	: m_parameters (parameters)
{}

MacQueue80211e::~MacQueue80211e ()
{}

void 
MacQueue80211e::enqueue (Packet *packet)
{
	cleanup ();
	double now;
	now = Scheduler::instance ().clock ();
	m_queue.push_front (pair<Packet *, double> (packet, now));
}
void 
MacQueue80211e::enqueueToHead (Packet *packet)
{
	cleanup ();
	double now;
	now = Scheduler::instance ().clock ();
	m_queue.push_back (pair<Packet *, double> (packet, now));
}

void
MacQueue80211e::cleanup (void)
{
	list <pair <Packet *, double> >::reverse_iterator tmp;
	double now;

	if (m_queue.empty ()) {
		return;
	}

	now = Scheduler::instance ().clock ();
	tmp = m_queue.rbegin ();
	while (tmp != m_queue.rend ()) {
		if ((*tmp).second + m_parameters->getMSDULifetime () > now) {
			break;
		}
		Packet::free ((*tmp).first);
		tmp++;
	}
	m_queue.erase (tmp.base (), m_queue.end ());
}

class Packet *
MacQueue80211e::dequeue (void)
{
	cleanup ();
	if (!m_queue.empty ()) {
		Packet *packet;
		packet = m_queue.back ().first;
		m_queue.pop_back ();
		return packet;
	}
	return NULL;
}

bool
MacQueue80211e::isEmpty (void)
{
	cleanup ();
	return m_queue.empty ();
}


int
MacQueue80211e::size (void)
{
	return m_queue.size ();
}

void
MacQueue80211e::flush (void)
{
	list <pair <Packet *, double> >::iterator tmp;
	for (tmp = m_queue.begin (); tmp != m_queue.end (); tmp++) {
		Packet::free ((*tmp).first);
		tmp++;
	}
	m_queue.erase (m_queue.begin (), m_queue.end ());
}
