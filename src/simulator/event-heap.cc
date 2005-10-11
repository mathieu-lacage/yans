/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "event-heap.h"
#include "clock.h"
#include "event.h"
#include <math.h>
#include <utility>

// XXX perform locking on list integrity.


EventHeap::EventHeap ()
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
}
void 
EventHeap::insert_at_s (Event *event, double time)
{
	assert (time > 0);
	long int us = lrint (time * 1000000);
	assert (us > 0);
	insert_at_us (event, (uint64_t)us);
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

void
EventHeap::wait (void)
{
	assert (false);
}
