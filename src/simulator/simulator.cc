/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "simulator.h"
#include "clock.h"
#include "event-heap.h"
#include "event-runnable.h"
#include "fiber.h"
#include "fiber-scheduler.h"

Simulator *Simulator::m_instance = 0;

Simulator *
Simulator::instance (void)
{
	if (m_instance == 0) {
		m_instance = new Simulator ();
	}
	return m_instance;
}

void
Simulator::destroy (void)
{
	delete m_instance;
	m_instance = 0;
}

Simulator::Simulator ()
{
	m_event_runnable = new EventRunnable ();
	m_event_fiber = new Fiber (0, m_event_runnable, "events");
}

Simulator::~Simulator ()
{
	m_event_runnable->stop ();
	//m_event_fiber->wait_until_is_dead ();
	delete m_event_fiber;
	delete m_event_runnable;
}

void 
Simulator::run (void)
{
	FiberScheduler::instance ()->run_main ();
}
void 
Simulator::stop (void)
{
	m_event_runnable->stop ();
}
void 
Simulator::insert_in_us (Event *event, uint64_t delta)
{
	m_event_runnable->insert_in_us (event, delta);
}
void 
Simulator::insert_at_us (Event *event, uint64_t time)
{
	m_event_runnable->insert_at_us (event, time);
}
uint64_t 
Simulator::now_us (void)
{
	return m_event_runnable->now_us ();
}
void 
Simulator::insert_in_s (Event *event, double delta)
{
	m_event_runnable->insert_in_s (event, delta);
}
void 
Simulator::insert_at_s (Event *event, double time)
{
	m_event_runnable->insert_at_s (event, time);
}
double 
Simulator::now_s (void)
{
	return m_event_runnable->now_s ();
}

