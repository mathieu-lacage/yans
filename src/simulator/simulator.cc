/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "simulator.h"
#include "clock.h"
#include "event-heap.h"
#include "event-runnable.h"
#include "fiber.h"

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
	m_clock = new Clock ();
	m_events = new EventHeap ();
	m_event_runnable = new EventRunnable ();
	m_event_runnable->set_simulator (this);
	m_event_fiber = new Fiber (m_event_runnable, "events");
}

Simulator::~Simulator ()
{
	delete m_clock;
	delete m_events;
	m_event_runnable->stop ();
	delete m_event_fiber;
	delete m_event_runnable;
}

void 
Simulator::run (void)
{
	
}
void 
Simulator::stop (void)
{
	m_event_runnable->stop ();
	m_events->clear ();
}
void 
Simulator::insert_in_us (Event *event, uint64_t delta)
{
	uint64_t current = m_clock->get_current_us ();
	insert_at_us (event, current+delta);
}
void 
Simulator::insert_at_us (Event *event, uint64_t time)
{
	m_events->insert_at_us (event, time);
}
uint64_t 
Simulator::now_us (void)
{
	return m_clock->get_current_us ();
}
void 
Simulator::insert_in_s (Event *event, double delta)
{
	double current = m_clock->get_current_s ();
	insert_at_s (event, current+delta);
}
void 
Simulator::insert_at_s (Event *event, double time)
{
	m_events->insert_at_s (event, time);
}
double 
Simulator::now_s (void)
{
	return m_clock->get_current_s ();
}
Clock *
Simulator::get_clock (void)
{
	return m_clock;
}
EventHeap *
Simulator::get_events (void)
{
	return m_events;
}

