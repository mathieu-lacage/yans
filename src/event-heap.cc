/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "event-heap.h"
#include "clock.h"
#include <math.h>
#include <utility>


EventHeap::EventHeap ()
{}
void 
EventHeap::insert_in_us (Event *event, uint64_t delta)
{
	uint64_t current = Clock::instance ()->get_current_us ();
	insert_at_us (event, current+delta);
}
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
EventHeap::insert_in_s (Event *event, double delta)
{
	double current = Clock::instance ()->get_current_s ();
	insert_at_s (event, current+delta);
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
	return m_events.front ().first;
}
uint64_t
EventHeap::peek_next_time (void)
{
	return m_events.front ().second;
}

void
EventHeap::remove_next (void)
{
	m_events.pop_front ();
}
