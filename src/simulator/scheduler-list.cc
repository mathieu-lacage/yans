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

#include "scheduler-list.h"
#include "clock.h"
#include "event.h"
#include <utility>
#include <cassert>

#define noTRACE_EVENT_LIST 1

#ifdef TRACE_EVENT_LIST
#include <iostream>
# define TRACE(x) \
std::cout << "LIST TRACE " << x << std::endl;
#else /* TRACE_EVENT_LIST */
# define TRACE(format,...)
#endif /* TRACE_EVENT_LIST */

namespace yans {


SchedulerList::SchedulerList ()
{}
SchedulerList::~SchedulerList ()
{}

/*  !! WARNING !! 
 * This is a very nasty piece of code but it really should work
 * with pretty much any implementation of a std::list.
 * it relies on the fact that a std::list<>::iterator has a single
 * member variable, a pointer.
 */
void 
SchedulerList::store_in_event (Event *ev, EventsI i)
{
	assert (sizeof (i) <= sizeof (Event));
	strncpy ((char *)&(ev->m_id), (char *)&i, sizeof (void *));
}
SchedulerList::EventsI 
SchedulerList::get_from_event (Event const*ev)
{
	SchedulerList::EventsI i;
	assert (sizeof (i) <= sizeof (Event));
	strncpy ((char *)&i, (char *)&(ev->m_id), sizeof (void *));
	return i;
}


Event * 
SchedulerList::insert_at_us (Event *event, uint64_t time)
{
	for (EventsI i = m_events.begin (); i != m_events.end (); i++) {
		if ((*i).second > time) {
			m_events.insert (i, std::make_pair (event, time));
			store_in_event (event, i);
			return event;
		}
	}
	m_events.push_back (std::make_pair (event, time));
	TRACE ("inserted " << time);
	print_debug ();
	store_in_event (event, --(m_events.end ()));
	return event;
}
Event *
SchedulerList::peek_next (void)
{
	if (m_events.empty ()) {
		return 0;
	}
	return m_events.front ().first;
}
uint64_t
SchedulerList::peek_next_time_us (void)
{
	if (m_events.empty ()) {
		return 0;
	}
	return m_events.front ().second;
}

void
SchedulerList::remove_next (void)
{
	m_events.pop_front ();
}

Event *
SchedulerList::remove (Event const*ev)
{
	m_events.erase (get_from_event (ev));
	return const_cast<Event *> (ev);
}

void 
SchedulerList::clear (void)
{
	/* We cannot really delete the items left in the event heap.
	 * Items are left only when the user calls Simulator::stop ()
	 */
	m_events.erase (m_events.begin (), m_events.end ());
}

void 
SchedulerList::print_debug (void)
{
	for (EventsI i = m_events.begin (); i != m_events.end (); i++) {
		TRACE ("time: " << (*i).second);
	}
	TRACE ("end");
}

}; // namespace yans
