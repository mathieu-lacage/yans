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

#include "event-binary-heap.h"

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

EventBinaryHeap::EventBinaryHeap ()
{
	// we purposedly waste an item at the start of
	// the array to make sure the indexes in the
	// array start at one.
	m_heap.push_back (std::make_pair ((EventHolder *)0, 0));
}

EventBinaryHeap::~EventBinaryHeap ()
{}

uint32_t 
EventBinaryHeap::parent (uint32_t id) const
{
	return id / 2;
}
uint32_t 
EventBinaryHeap::sibling (uint32_t id) const
{
	return id + 1;
}
uint32_t 
EventBinaryHeap::left_child (uint32_t id) const
{
	return id * 2;
}
uint32_t 
EventBinaryHeap::right_child (uint32_t id) const
{
	return id * 2 + 1;
}

uint32_t
EventBinaryHeap::root (void) const
{
	return 1;
}

bool
EventBinaryHeap::is_root (uint32_t id) const
{
	return (id == root ())?true:false;
}

uint32_t
EventBinaryHeap::last (void) const
{
	return m_heap.size () - 1;
}


bool
EventBinaryHeap::is_bottom (uint32_t id) const
{
	return (id >= m_heap.size ())?true:false;
}

void
EventBinaryHeap::exch (uint32_t a, uint32_t b) 
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
EventBinaryHeap::is_less (uint32_t a, uint32_t b)
{
	return (m_heap[a].second < m_heap[b].second)?true:false;
}

uint32_t 
EventBinaryHeap::smallest (uint32_t a, uint32_t b)
{
	return is_less (a,b)?a:b;
}

bool
EventBinaryHeap::is_empty (void) const
{
	return (m_heap.size () == 1)?true:false;
}

void
EventBinaryHeap::bottom_up (void)
{
	uint32_t index = last ();
	while (!is_root (index) && 
	       is_less (index, parent (index))) { 
		exch(index, parent (index)); 
		index = parent (index); 
	}
}

void
EventBinaryHeap::top_down (void)
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
EventBinaryHeap::insert_at_us (Event *event, uint64_t time)
{
	EventHolder *holder = new EventHolder (event);
	m_heap.push_back (std::make_pair (holder, time));
	holder->reset_index (last ());
	bottom_up ();
	return EventIdHeap (holder);
}

Event   *
EventBinaryHeap::peek_next (void)
{
	if (is_empty ()) {
		return 0;
	}
	return m_heap[root ()].first->get_event ();
}
uint64_t 
EventBinaryHeap::peek_next_time_us (void)
{
	if (is_empty ()) {
		return 0;
	}
	return m_heap[root ()].second;
}
void     
EventBinaryHeap::remove_next (void)
{
	assert (!is_empty ());
	exch (root (), last ());
	delete m_heap.back ().first;
	m_heap.pop_back ();
	top_down ();
}

Event *
EventBinaryHeap::remove (EventId fake_id)
{
	EventIdHeap id = EventIdHeap (fake_id);
	exch (id.get_holder ()->get_index (), last ());
	delete m_heap.back ().first;
	m_heap.pop_back ();
	top_down ();
	return 0;
}

void 
EventBinaryHeap::clear (void)
{}
void 
EventBinaryHeap::print_debug (void)
{}

}; // namespace yans
