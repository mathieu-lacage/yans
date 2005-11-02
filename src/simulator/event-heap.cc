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

#include "event-heap.h"
#include "clock.h"
#include "event.h"
#include <utility>

EventHeap::EventHeap ()
{}
EventHeap::~EventHeap ()
{}
void 
EventHeap::insert_at_us (Event *event, uint64_t time)
{
	for (EventsI i = m_events.begin (); i != m_events.end (); i++) {
		if ((*i).second > time) {
			m_events.insert (i, std::make_pair (event, time));
			break;
		}
	}
	m_events.push_back (std::make_pair (event, time));
}
Event *
EventHeap::peek_next (void)
{
	if (m_events.empty ()) {
		return 0;
	}
	return m_events.front ().first;
}
uint64_t
EventHeap::peek_next_time_us (void)
{
	if (m_events.empty ()) {
		return 0;
	}
	return m_events.front ().second;
}

void
EventHeap::remove_next (void)
{
	m_events.pop_front ();
}

void 
EventHeap::clear (void)
{
	for (EventsI i = m_events.begin (); i != m_events.end (); i++) {
		(*i).first->notify_canceled ();
	}
	m_events.erase (m_events.begin (), m_events.end ());
}
