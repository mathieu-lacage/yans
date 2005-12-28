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
#include "event-runnable.h"
#include "fiber.h"
#include "fiber-scheduler.h"
#include "tag-manager.h"

namespace yans {

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

private:
	EventRunnable *m_event_runnable;
	Fiber *m_event_fiber;
};

SimulatorPrivate::SimulatorPrivate ()
{
	m_event_runnable = new EventRunnable ();
	m_event_fiber = new Fiber (0, m_event_runnable, "events");
}

SimulatorPrivate::~SimulatorPrivate ()
{
	m_event_runnable->stop ();
	FiberScheduler::instance ()->run_main ();
	delete m_event_fiber;
	delete m_event_runnable;
	FiberScheduler::instance ()->destroy ();
	TagManager::instance ()->destroy ();
}

void
SimulatorPrivate::run (void)
{
	FiberScheduler::instance ()->run_main ();
}

void 
SimulatorPrivate::stop (void)
{
	m_event_runnable->stop ();
}
void 
SimulatorPrivate::insert_in_us (Event *event, uint64_t delta)
{
	m_event_runnable->insert_in_us (event, delta);
}
void 
SimulatorPrivate::insert_at_us (Event *event, uint64_t time)
{
	m_event_runnable->insert_at_us (event, time);
}
uint64_t 
SimulatorPrivate::now_us (void)
{
	return m_event_runnable->now_us ();
}
void 
SimulatorPrivate::insert_in_s (Event *event, double delta)
{
	m_event_runnable->insert_in_s (event, delta);
}
void 
SimulatorPrivate::insert_at_s (Event *event, double time)
{
	m_event_runnable->insert_at_s (event, time);
}
double 
SimulatorPrivate::now_s (void)
{
	return m_event_runnable->now_s ();
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

}; // namespace yans
