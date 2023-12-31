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
#include "scheduler.h"
#include "event.h"
#include "event.tcc"
#include "yans/system-semaphore.h"

#include <math.h>
#include <cassert>
#include <fstream>
#include <list>
#include <vector>
#include <iostream>

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

class ParallelSimulatorQueuePrivate {
public:
	ParallelSimulatorQueuePrivate (SimulatorPrivate *priv);
	~ParallelSimulatorQueuePrivate ();
	void set_queue (ParallelSimulatorQueue *queue);
	void schedule_abs_us (Event ev, uint64_t at);
	void send_null_message (void);
private:
	void remove_event (Event ev);
	uint32_t m_n;
	SimulatorPrivate *m_simulator;
	ParallelSimulatorQueue *m_queue;
};

class SimulatorPrivate {
public:
	SimulatorPrivate (Scheduler *events);
	~SimulatorPrivate ();

	void enable_log_to (char const *filename);

	void add_queue (ParallelSimulatorQueuePrivate *queue);
	void notify_queue_not_empty (void);
	void notify_queue_empty (void);
	void wait_until_no_queue_empty (void);
	bool is_parallel (void);

	bool is_finished (void) const;
	uint64_t next_us (void) const;
	void run_serial (void);
	void run_parallel (void);
	void stop (void);
	void stop_at_us (uint64_t at);
	Event schedule_rel_us (Event event, uint64_t delta);
	Event schedule_rel_s (Event event, double delta);
	Event schedule_abs_us (Event event, uint64_t time);
	Event schedule_abs_s (Event event, double time);
	Event remove (Event const ev);
	uint64_t now_us (void);
	double now_s (void);
	void schedule_now (Event event);
	void schedule_destroy (Event event);

private:
	void process_one_event (void);

	typedef std::list<std::pair<Event,uint32_t> > Events;
	typedef std::vector<ParallelSimulatorQueuePrivate *> Queues;
	typedef std::vector<ParallelSimulatorQueuePrivate *>::iterator QueuesI;
	Events m_destroy;
	uint64_t m_stop_at;
	bool m_stop;
	Scheduler *m_events;
	uint32_t m_uid;
	uint32_t m_current_uid;
	uint64_t m_current_us;
	std::ofstream m_log;
	std::ifstream m_input_log;
	bool m_log_enable;
	uint32_t m_n_full_queues;
	uint32_t m_n_queues;
	SystemSemaphore *m_all_queues;
	Queues m_queues;
};


ParallelSimulatorQueuePrivate::ParallelSimulatorQueuePrivate (SimulatorPrivate *simulator)
	: m_simulator (simulator)
{}
ParallelSimulatorQueuePrivate::~ParallelSimulatorQueuePrivate ()
{}
void 
ParallelSimulatorQueuePrivate::schedule_abs_us (Event ev, uint64_t at)
{
	m_n++;
	if (m_n == 1) {
		m_simulator->notify_queue_not_empty ();
	}
	m_simulator->schedule_abs_us (make_event (&ParallelSimulatorQueuePrivate::remove_event, this, ev), at);
}
void
ParallelSimulatorQueuePrivate::remove_event (Event ev)
{
	m_n--;
	if (m_n == 0) {
		m_simulator->notify_queue_empty ();
	}
	ev ();
}

void
ParallelSimulatorQueuePrivate::set_queue (ParallelSimulatorQueue *queue)
{
	m_queue = queue;
}

void 
ParallelSimulatorQueuePrivate::send_null_message (void)
{
	m_queue->send_null_message ();
}




SimulatorPrivate::SimulatorPrivate (Scheduler *events)
{
	m_stop = false;
	m_stop_at = 0;
	m_events = events;
	m_uid = 0;	
	m_log_enable = false;
	m_current_us = 0;
	m_all_queues = new SystemSemaphore (0);
	m_n_queues = 0;
	m_n_full_queues = 0;
}

SimulatorPrivate::~SimulatorPrivate ()
{
	while (!m_destroy.empty ()) {
		Event ev = m_destroy.front ().first;
		m_destroy.pop_front ();
		TRACE ("handle destroy " << ev);
		ev ();
	}
	delete m_events;
	m_events = (Scheduler *)0xdeadbeaf;
	delete m_all_queues;
	m_queues.erase (m_queues.begin (), m_queues.end ());
}

bool 
SimulatorPrivate::is_parallel (void)
{
	return (m_n_queues > 0);
}

void
SimulatorPrivate::enable_log_to (char const *filename)
{
	m_log.open (filename);
	m_log_enable = true;
}

void 
SimulatorPrivate::notify_queue_not_empty (void)
{
	m_n_full_queues++;
	if (m_n_full_queues == m_n_queues) {
		m_all_queues->post ();
	}
}
void 
SimulatorPrivate::notify_queue_empty (void)
{
	m_n_full_queues--;
}
void
SimulatorPrivate::wait_until_no_queue_empty (void)
{
	while (m_n_full_queues < m_n_queues) {
		m_all_queues->wait ();
	}
}

void
SimulatorPrivate::add_queue (ParallelSimulatorQueuePrivate *queue)
{
	m_n_queues++;
	m_queues.push_back (queue);
}

void
SimulatorPrivate::process_one_event (void)
{
	Event next_ev = m_events->peek_next ();
	Scheduler::EventKey next_key = m_events->peek_next_key ();
	m_events->remove_next ();
	TRACE ("handle " << next_ev);
	m_current_us = next_key.m_time;
	m_current_uid = next_key.m_uid;
	if (m_log_enable) {
		m_log << "e "<<next_key.m_uid << " " << next_key.m_time << std::endl;
	}
	next_ev ();
}

bool 
SimulatorPrivate::is_finished (void) const
{
	return m_events->is_empty ();
}
uint64_t 
SimulatorPrivate::next_us (void) const
{
	assert (!m_events->is_empty ());
	Scheduler::EventKey next_key = m_events->peek_next_key ();
	return next_key.m_time;
}


void
SimulatorPrivate::run_serial (void)
{
	while (!m_events->is_empty () && !m_stop && 
	       (m_stop_at == 0 || m_stop_at > next_us ())) {
		process_one_event ();
	}
	m_log.close ();
}

void
SimulatorPrivate::run_parallel (void)
{
	TRACE ("run parallel");
	while (!m_stop && 
	       (m_stop_at == 0 || m_stop_at >= next_us ())) {
		TRACE ("send null messages");
		for (QueuesI i = m_queues.begin (); i != m_queues.end (); i++) {
			(*i)->send_null_message ();
		}
		TRACE ("sent null messages");
		wait_until_no_queue_empty ();
		TRACE ("no queue empty");
		process_one_event();
		TRACE ("processed event");
	}
	m_log.close ();
	TRACE ("done run parallel");
}

void 
SimulatorPrivate::stop (void)
{
	m_stop = true;
}
void 
SimulatorPrivate::stop_at_us (uint64_t at)
{
	m_stop_at = at;
}
Event   
SimulatorPrivate::schedule_rel_us (Event event, uint64_t delta)
{
	uint64_t current = now_us ();
	return schedule_abs_us (event, current+delta);
}
Event  
SimulatorPrivate::schedule_abs_us (Event event, uint64_t time)
{
	assert (time >= now_us ());
	Scheduler::EventKey key = {time, m_uid};
	if (m_log_enable) {
		m_log << "i "<<m_current_uid<<" "<<now_us ()<<" "
		      <<m_uid<<" "<<time << std::endl;
	}
	m_uid++;
	return m_events->insert (event, key);
}
uint64_t 
SimulatorPrivate::now_us (void)
{
	return m_current_us;
}
Event  
SimulatorPrivate::schedule_rel_s (Event event, double delta)
{
	int64_t delta_us = (int64_t)(delta * 1000000.0);
	uint64_t us = now_us () + delta_us;
	return schedule_abs_us (event, us);
}
Event  
SimulatorPrivate::schedule_abs_s (Event event, double time)
{
	int64_t us = (int64_t)(time * 1000000.0);
	assert (us >= 0);
	return schedule_abs_us (event, (uint64_t)us);
}
double 
SimulatorPrivate::now_s (void)
{
	double us = m_current_us;
	us /= 1000000;
	return us;
}
void
SimulatorPrivate::schedule_now (Event event)
{
	schedule_abs_us (event, now_us ());
}
void
SimulatorPrivate::schedule_destroy (Event event)
{
	m_destroy.push_back (std::make_pair (event, m_uid));
	if (m_log_enable) {
		m_log << "id " << m_current_uid << " " << now_us () << " "
		      << m_uid << std::endl;
	}
	m_uid++;
}

Event 
SimulatorPrivate::remove (Event const ev)
{
	Scheduler::EventKey key = m_events->remove (ev);
	if (m_log_enable) {
		m_log << "r " << m_current_uid << " " << now_us () << " "
		      << key.m_uid << " " << key.m_time << std::endl;
	}
	return Event (ev);
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
void Simulator::enable_log_to (char const *filename)
{
	get_priv ()->enable_log_to (filename);
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
Simulator::add_parallel_queue (ParallelSimulatorQueue *queue)
{
	ParallelSimulatorQueuePrivate *priv = new ParallelSimulatorQueuePrivate (get_priv ());
	priv->set_queue (queue);
	queue->set_priv (priv);
	return get_priv ()->add_queue (priv);
}

bool 
Simulator::is_finished (void)
{
	return get_priv ()->is_finished ();
}
uint64_t 
Simulator::next_us (void)
{
	return get_priv ()->next_us ();
}


void 
Simulator::run (void)
{
	if (get_priv ()->is_parallel ()) {
		get_priv ()->run_parallel ();
	} else {
		get_priv ()->run_serial ();
	}
}
void 
Simulator::stop (void)
{
	TRACE ("stop");
	get_priv ()->stop ();
}
void 
Simulator::stop_at_us (uint64_t at)
{
	get_priv ()->stop_at_us (at);
}
Event 
Simulator::schedule_rel_us (uint64_t delta, Event event)
{
	TRACE ("insert " << event << " in " << delta << "us");
	return get_priv ()->schedule_rel_us (event, delta);
}
Event 
Simulator::schedule_abs_us (uint64_t time, Event event)
{
	TRACE ("insert " << event << " at " << time << "us");
	return get_priv ()->schedule_abs_us (event, time);
}
uint64_t 
Simulator::now_us (void)
{
	return get_priv ()->now_us ();
}
Event  
Simulator::schedule_rel_s (double delta, Event event)
{
	TRACE ("insert " << event << " in " << delta << "s");
	return get_priv ()->schedule_rel_s (event, delta);
}
Event  
Simulator::schedule_abs_s (double time, Event event)
{
	TRACE ("insert " << event << " at " << time << "s");
	return get_priv ()->schedule_abs_s (event, time);
}
double 
Simulator::now_s (void)
{
	return get_priv ()->now_s ();
}
void
Simulator::schedule_now (Event event)
{
	TRACE ("insert later " << event);
	return get_priv ()->schedule_now (event);
}
void 
Simulator::schedule_destroy (Event event)
{
	TRACE ("insert at destroy " << event);
	return get_priv ()->schedule_destroy (event);
}

Event 
Simulator::remove (Event const ev)
{
	return get_priv ()->remove (ev);
}

}; // namespace yans


namespace yans {

ParallelSimulatorQueue::ParallelSimulatorQueue ()
{}
ParallelSimulatorQueue::~ParallelSimulatorQueue ()
{
	delete m_priv;
}
void 
ParallelSimulatorQueue::schedule_abs_us (uint64_t at, Event ev)
{
	m_priv->schedule_abs_us (ev, at);
}
void
ParallelSimulatorQueue::set_priv (ParallelSimulatorQueuePrivate *priv)
{
	m_priv = priv;
}
};
