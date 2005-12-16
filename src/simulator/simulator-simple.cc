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

#include "simulator.h"
#include "clock.h"
#include "event-heap.h"
#include "tag-manager.h"
#include "event.h"
#include <math.h>

class SimulatorPrivate {
public:
	SimulatorPrivate ();
	~SimulatorPrivate ();

	void run (void);
	void stop (void);
	void insert_in_us (Event *event, uint64_t delta);
	void insert_at_us (Event *event, uint64_t time);
	uint64_t now_us (void);
	void insert_in_s (Event *event, double delta);
	void insert_at_s (Event *event, double time);
	double now_s (void);
	void insert_later (Event *event);

private:
	void handle_immediate (void);
	typedef std::list<Event *> Events;
	Events m_immediate;
	bool m_stop;
	Clock *m_clock;
	EventHeap *m_event_heap;
};

SimulatorPrivate::SimulatorPrivate ()
{
	m_stop = false;
	m_clock = new Clock ();
	m_event_heap = new EventHeap ();
}

SimulatorPrivate::~SimulatorPrivate ()
{
	delete m_clock;
	m_clock = (Clock *)0xdeadbeaf;
	delete m_event_heap;
	m_event_heap = (EventHeap *)0xdeadbeaf;
	TagManager::instance ()->destroy ();
}

void
SimulatorPrivate::handle_immediate (void)
{
	while (!m_immediate.empty ()) {
		Event *ev = m_immediate.front ();
		m_immediate.pop_front ();
		ev->notify ();
	}
}

void
SimulatorPrivate::run (void)
{
	Event *next_ev = m_event_heap->peek_next ();
	uint64_t next_now = m_event_heap->peek_next_time_us ();
	handle_immediate ();
	while (next_ev != 0 && !m_stop) {
		m_event_heap->remove_next ();
		m_clock->update_current_us (next_now);
		next_ev->notify ();
		handle_immediate ();
		next_ev = m_event_heap->peek_next ();
		next_now = m_event_heap->peek_next_time_us ();
	}
}

void 
SimulatorPrivate::stop (void)
{
	m_stop = true;
}
void 
SimulatorPrivate::insert_in_us (Event *event, uint64_t delta)
{
	uint64_t current = m_clock->get_current_us ();
	m_event_heap->insert_at_us (event, current+delta);
}
void 
SimulatorPrivate::insert_at_us (Event *event, uint64_t time)
{
	m_event_heap->insert_at_us (event, time);
}
uint64_t 
SimulatorPrivate::now_us (void)
{
	return m_clock->get_current_us ();
}
void 
SimulatorPrivate::insert_in_s (Event *event, double delta)
{
	double current = m_clock->get_current_s ();
	insert_at_s (event, current+delta);
}
void 
SimulatorPrivate::insert_at_s (Event *event, double time)
{
	assert (time > 0);
	long int us = lrint (time * 1000000.0);
	assert (us > 0);
	m_event_heap->insert_at_us (event, (uint64_t)us);
}
double 
SimulatorPrivate::now_s (void)
{
	return m_clock->get_current_s ();
}
void
SimulatorPrivate::insert_later (Event *event)
{
	m_immediate.push_back (event);
}










bool Simulator::m_destroyed = false;


SimulatorPrivate *
Simulator::get_priv (void)
{
	assert (!m_destroyed);
	static SimulatorPrivate *priv = new SimulatorPrivate ();
	return priv;
}

void
Simulator::destroy (void)
{
	delete get_priv ();
	m_destroyed = true;
}

void 
Simulator::run (void)
{
	get_priv ()->run ();
}
void 
Simulator::stop (void)
{
	get_priv ()->stop ();
}
void 
Simulator::insert_in_us (uint64_t delta, Event *event)
{
	get_priv ()->insert_in_us (event, delta);
}
void 
Simulator::insert_at_us (uint64_t time, Event *event)
{
	get_priv ()->insert_at_us (event, time);
}
uint64_t 
Simulator::now_us (void)
{
	return get_priv ()->now_us ();
}
void 
Simulator::insert_in_s (double delta, Event *event)
{
	get_priv ()->insert_in_s (event, delta);
}
void 
Simulator::insert_at_s (double time, Event *event)
{
	get_priv ()->insert_at_s (event, time);
}
double 
Simulator::now_s (void)
{
	return get_priv ()->now_s ();
}
void
Simulator::insert_later (Event *event)
{
	return get_priv ()->insert_later (event);
}
