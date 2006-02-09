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

#include "event-list.h"
#include "clock.h"
#include "event.h"
#include <utility>

#define noTRACE_EVENT_LIST 1

#ifdef TRACE_EVENT_LIST
#include <iostream>
# define TRACE(x) \
std::cout << "LIST TRACE " << x << std::endl;
#else /* TRACE_EVENT_LIST */
# define TRACE(format,...)
#endif /* TRACE_EVENT_LIST */

namespace yans {

/*  !! WARNING !! 
 * This is a very nasty piece of code but it really should work
 * with pretty much any implementation of a std::list.
 * it relies on the fact that a std::list<>::iterator has a single
 * member variable, a pointer.
 */
class EventListId : public EventId {
public:
	EventListId (EventId id) : EventId (id) {}
	EventListId (EventList::EventsI i) {
		assert (sizeof (i) <= sizeof (EventId));
		strncpy ((char *)&m_id, (char *)&i, sizeof (i));
	}
	EventList::EventsI get_iterator (void) {
		EventList::EventsI i;
		assert (sizeof (i) <= sizeof (EventId));
		strncpy ((char *)&i, (char *)&m_id, sizeof (i));
		return i;
	}
};

EventList::EventList ()
{}
EventList::~EventList ()
{}
EventId 
EventList::insert_at_us (Event *event, uint64_t time)
{
	for (EventsI i = m_events.begin (); i != m_events.end (); i++) {
		if ((*i).second > time) {
			m_events.insert (i, std::make_pair (event, time));
			return EventListId (i);
		}
	}
	m_events.push_back (std::make_pair (event, time));
	TRACE ("inserted " << time);
	print_debug ();
	return EventListId (--(m_events.end ()));
}
Event *
EventList::peek_next (void)
{
	if (m_events.empty ()) {
		return 0;
	}
	return m_events.front ().first;
}
uint64_t
EventList::peek_next_time_us (void)
{
	if (m_events.empty ()) {
		return 0;
	}
	return m_events.front ().second;
}

void
EventList::remove_next (void)
{
	m_events.pop_front ();
}

Event *
EventList::remove (EventId id)
{
	EventListId real_id (id);
	Event *event = (*(real_id.get_iterator ())).first;
	m_events.erase (real_id.get_iterator ());
	return event;
}

void 
EventList::clear (void)
{
	/* We cannot really delete the items left in the event heap.
	 * Items are left only when the user calls Simulator::stop ()
	 */
	m_events.erase (m_events.begin (), m_events.end ());
}

void 
EventList::print_debug (void)
{
	for (EventsI i = m_events.begin (); i != m_events.end (); i++) {
		TRACE ("time: " << (*i).second);
	}
	TRACE ("end");
}

}; // namespace yans
