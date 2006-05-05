/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "scheduler.h"
#include "event.h"
#include <math.h>
#include <cassert>
#include <list>

#define noTRACE_SIMU 1

#ifdef TRACE_SIMU
#include <iostream>
# define TRACE(x) \
std::cout << "SIMU TRACE " << Simulator::now_s () << " " << x << std::endl;
# define TRACE_S(x) \
std::cout << "SIMU TRACE " << x << std::endl;
#else /* TRACE_SIMU */
# define TRACE(format,...)
# define TRACE_S(format,...)
#endif /* TRACE_SIMU */


namespace yans {

class SimulatorPrivate {
public:
	SimulatorPrivate (Scheduler *events);
	~SimulatorPrivate ();

	void run (void);
	void stop (void);
	Event *insert_in_us (Event *event, uint64_t delta);
	Event *insert_in_s (Event *event, double delta);
	Event *insert_at_us (Event *event, uint64_t time);
	Event *insert_at_s (Event *event, double time);
	Event *remove (Event const*ev);
	uint64_t now_us (void);
	double now_s (void);
	void insert_later (Event *event);
	void insert_at_destroy (Event *event);

private:
	void handle_immediate (void);
	typedef std::list<Event *> Events;
	Events m_immediate;
	Events m_destroy;
	bool m_stop;
	Clock *m_clock;
	Scheduler *m_events;
};

SimulatorPrivate::SimulatorPrivate (Scheduler *events)
{
	m_stop = false;
	m_clock = new Clock ();
	m_events = events;
}

SimulatorPrivate::~SimulatorPrivate ()
{
	while (!m_destroy.empty ()) {
		Event *ev = m_destroy.front ();
		m_destroy.pop_front ();
		TRACE ("handle destroy " << ev);
		ev->notify ();
	}
	delete m_clock;
	m_clock = (Clock *)0xdeadbeaf;
	delete m_events;
	m_events = (Scheduler *)0xdeadbeaf;
}

void
SimulatorPrivate::handle_immediate (void)
{
	while (!m_immediate.empty ()) {
		Event *ev = m_immediate.front ();
		m_immediate.pop_front ();
		TRACE ("handle imm " << ev);
		ev->notify ();
	}
}

void
SimulatorPrivate::run (void)
{
	handle_immediate ();
	Event *next_ev = m_events->peek_next ();
	uint64_t next_now = m_events->peek_next_time_us ();
	while (next_ev != 0 && !m_stop) {
		m_events->remove_next ();
		m_clock->update_current_us (next_now);
		TRACE ("handle " << next_ev);
		next_ev->notify ();
		handle_immediate ();
		next_ev = m_events->peek_next ();
		next_now = m_events->peek_next_time_us ();
	}
}

void 
SimulatorPrivate::stop (void)
{
	m_stop = true;
}
Event *  
SimulatorPrivate::insert_in_us (Event *event, uint64_t delta)
{
	uint64_t current = m_clock->get_current_us ();
	return m_events->insert_at_us (event, current+delta);
}
Event * 
SimulatorPrivate::insert_at_us (Event *event, uint64_t time)
{
	assert (time >= m_clock->get_current_us ());
	return m_events->insert_at_us (event, time);
}
uint64_t 
SimulatorPrivate::now_us (void)
{
	return m_clock->get_current_us ();
}
Event * 
SimulatorPrivate::insert_in_s (Event *event, double delta)
{
	uint64_t now_us = m_clock->get_current_us ();
	int64_t delta_us = (int64_t)(delta * 1000000.0);
	uint64_t us = now_us + delta_us;
	return m_events->insert_at_us (event, us);
}
Event * 
SimulatorPrivate::insert_at_s (Event *event, double time)
{
	int64_t us = (int64_t)(time * 1000000.0);
	assert (us >= 0);
	return m_events->insert_at_us (event, (uint64_t)us);
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
void
SimulatorPrivate::insert_at_destroy (Event *event)
{
	m_destroy.push_back (event);
}

Event *
SimulatorPrivate::remove (Event const*ev)
{
	return m_events->remove (ev);
}


}; // namespace yans


#include "scheduler-list.h"
#include "scheduler-heap.h"
#include "scheduler-map.h"


namespace yans {

SimulatorPrivate *Simulator::m_priv = 0;
Simulator::ListType Simulator::m_list_type = LINKED_LIST;

void Simulator::set_linked_list (void)
{
	m_list_type = LINKED_LIST;
}
void Simulator::set_binary_heap (void)
{
	m_list_type = BINARY_HEAP;
}
void Simulator::set_std_map (void)
{
	m_list_type = STD_MAP;
}


SimulatorPrivate *
Simulator::get_priv (void)
{
	if (m_priv == 0) {
		Scheduler *events;
		switch (m_list_type) {
		case LINKED_LIST:
			events = new SchedulerList ();
			break;
		case BINARY_HEAP:
			events = new SchedulerHeap ();
			break;
		case STD_MAP:
			events = new SchedulerMap ();
			break;
		default: // not reached
			events = 0;
			assert (false); 
			break;
		}
		m_priv = new SimulatorPrivate (events);
	}
	TRACE_S ("priv " << m_priv);
	return m_priv;
}

void
Simulator::destroy (void)
{
	delete m_priv;
	m_priv = 0;
}

void 
Simulator::run (void)
{
	get_priv ()->run ();
}
void 
Simulator::stop (void)
{
	TRACE ("stop");
	get_priv ()->stop ();
}
Event *
Simulator::insert_in_us (uint64_t delta, Event *event)
{
	TRACE ("insert " << event << " in " << delta << "us");
	return get_priv ()->insert_in_us (event, delta);
}
Event *
Simulator::insert_at_us (uint64_t time, Event *event)
{
	TRACE ("insert " << event << " at " << time << "us");
	return get_priv ()->insert_at_us (event, time);
}
uint64_t 
Simulator::now_us (void)
{
	return get_priv ()->now_us ();
}
Event * 
Simulator::insert_in_s (double delta, Event *event)
{
	TRACE ("insert " << event << " in " << delta << "s");
	return get_priv ()->insert_in_s (event, delta);
}
Event * 
Simulator::insert_at_s (double time, Event *event)
{
	TRACE ("insert " << event << " at " << time << "s");
	return get_priv ()->insert_at_s (event, time);
}
double 
Simulator::now_s (void)
{
	return get_priv ()->now_s ();
}
void
Simulator::insert_later (Event *event)
{
	TRACE ("insert later " << event);
	return get_priv ()->insert_later (event);
}
void 
Simulator::insert_at_destroy (Event *event)
{
	TRACE ("insert at destroy " << event);
	return get_priv ()->insert_at_destroy (event);
}

Event *
Simulator::remove (Event const*ev)
{
	return get_priv ()->remove (ev);
}

}; // namespace yans
