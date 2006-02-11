/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
 * Copyright (c) 2005 Mathieu Lacage
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
 *
 * This code started as a c++ translation of a java-based code written in 2005
 * to implement a heap sort. Which explains the Copyright Mathieu Lacage at the
 * top of this file.
 *
 * What is smart about this code ?
 *  - it does not use the index 0 in the array to avoid having to convert
 *    C-style array indexes (which start at zero) and heap-style indexes
 *    (which start at 1). This is why _all_ indexes start at 1, and that
 *    the index of the root is 1.
 *  - It uses a slightly non-standard while loop for top-down heapify
 *    to move one if statement out of the loop.
 */

#include "scheduler-heap.h"

#define noTRACE_HEAP 1

#ifdef TRACE_HEAP
#include <iostream>
# define TRACE(x) \
std::cout << "HEAP TRACE " << x << std::endl;
#else /* TRACE_HEAP */
# define TRACE(format,...)
#endif /* TRACE_HEAP */


namespace yans {

class EventIdHeap : public EventId {
public:
	EventIdHeap (EventHolder *holder) 
		: EventId (holder) {}
	EventIdHeap (EventId id)
		: EventId (id) {}
	EventHolder *get_holder (void) {
		return reinterpret_cast<EventHolder *> (EventId::get_id ());
	}
};

class EventHolder {
public:
	EventHolder (Event *event) 
		: m_event (event), m_index ()
	{}
	~EventHolder () {
		m_event = (Event *)0xdeadbeaf;
		m_index = 0xdeadbeaf;
	}
	Event *get_event (void) {
		return m_event;
	}
	uint32_t get_index (void) {
		return m_index;
	}
	void reset_index (uint32_t index) {
		m_index = index;
	}
private:
	Event *m_event;
	uint32_t m_index;
};

SchedulerHeap::SchedulerHeap ()
{
	// we purposedly waste an item at the start of
	// the array to make sure the indexes in the
	// array start at one.
	m_heap.push_back (std::make_pair ((EventHolder *)0, 0));
}

SchedulerHeap::~SchedulerHeap ()
{}

uint32_t 
SchedulerHeap::parent (uint32_t id) const
{
	return id / 2;
}
uint32_t 
SchedulerHeap::sibling (uint32_t id) const
{
	return id + 1;
}
uint32_t 
SchedulerHeap::left_child (uint32_t id) const
{
	return id * 2;
}
uint32_t 
SchedulerHeap::right_child (uint32_t id) const
{
	return id * 2 + 1;
}

uint32_t
SchedulerHeap::root (void) const
{
	return 1;
}

bool
SchedulerHeap::is_root (uint32_t id) const
{
	return (id == root ())?true:false;
}

uint32_t
SchedulerHeap::last (void) const
{
	return m_heap.size () - 1;
}


bool
SchedulerHeap::is_bottom (uint32_t id) const
{
	return (id >= m_heap.size ())?true:false;
}

void
SchedulerHeap::exch (uint32_t a, uint32_t b) 
{
	assert (b < m_heap.size () && a < m_heap.size ());
	TRACE ("exch " << a << ", " << b);
	std::pair<EventHolder *, uint64_t> tmp (m_heap[a]);
	m_heap[a] = m_heap[b];
	m_heap[b] = tmp;
	m_heap[b].first->reset_index (b);
	m_heap[a].first->reset_index (a);
}

bool
SchedulerHeap::is_less (uint32_t a, uint32_t b)
{
	return (m_heap[a].second < m_heap[b].second)?true:false;
}

uint32_t 
SchedulerHeap::smallest (uint32_t a, uint32_t b)
{
	return is_less (a,b)?a:b;
}

bool
SchedulerHeap::is_empty (void) const
{
	return (m_heap.size () == 1)?true:false;
}

void
SchedulerHeap::bottom_up (void)
{
	uint32_t index = last ();
	while (!is_root (index) && 
	       is_less (index, parent (index))) { 
		exch(index, parent (index)); 
		index = parent (index); 
	}
}

void
SchedulerHeap::top_down (void)
{
	uint32_t index = root ();
	uint32_t right = right_child (index);
	while (!is_bottom (right)) {
		uint32_t left = left_child (index);
		uint32_t tmp = smallest (left, right);
		if (is_less (index, tmp)) {
			return;
		}
		exch (index, tmp);
		index = tmp;
		right = right_child (index);
	}
	if (is_bottom (index)) {
		return;
	}
	assert (!is_bottom (index));
	uint32_t left = left_child (index);
	if (is_bottom (left)) {
		return;
	}
	if (is_less (index, left)) {
		return;
	}
	exch (index, left);
}


EventId 
SchedulerHeap::insert_at_us (Event *event, uint64_t time)
{
	EventHolder *holder = new EventHolder (event);
	m_heap.push_back (std::make_pair (holder, time));
	holder->reset_index (last ());
	bottom_up ();
	return EventIdHeap (holder);
}

Event   *
SchedulerHeap::peek_next (void)
{
	if (is_empty ()) {
		return 0;
	}
	return m_heap[root ()].first->get_event ();
}
uint64_t 
SchedulerHeap::peek_next_time_us (void)
{
	if (is_empty ()) {
		return 0;
	}
	return m_heap[root ()].second;
}
void     
SchedulerHeap::remove_next (void)
{
	assert (!is_empty ());
	exch (root (), last ());
	delete m_heap.back ().first;
	m_heap.pop_back ();
	top_down ();
}

Event *
SchedulerHeap::remove (EventId fake_id)
{
	EventIdHeap id = EventIdHeap (fake_id);
	exch (id.get_holder ()->get_index (), last ());
	delete m_heap.back ().first;
	m_heap.pop_back ();
	top_down ();
	return 0;
}

void 
SchedulerHeap::clear (void)
{}
void 
SchedulerHeap::print_debug (void)
{}

}; // namespace yans
