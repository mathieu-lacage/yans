/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "event-runnable.h"
#include "clock.h"
#include "event-heap.h"
#include "event.h"
#include "simulator.h"
#include "fiber-scheduler.h"

EventRunnable::EventRunnable ()
{
	m_stop = false;
}
void
EventRunnable::set_simulator (Simulator *simulator)
{
	m_simulator = simulator;
}
void
EventRunnable::stop (void)
{
	m_stop = true;
}
void 
EventRunnable::run (void)
{
	Event *ev = m_simulator->get_events ()->peek_next ();
	uint64_t now = m_simulator->get_events ()->peek_next_time_us ();
	while (!m_stop) {
		while (ev != 0 && !m_stop) {
			m_simulator->get_clock ()->update_current_us (now);
			ev->notify ();
			EventHeap *events = m_simulator->get_events ();
			ev = events->peek_next ();
			now = events->peek_next_time_us ();
			FiberScheduler::instance ()->schedule ();
		}
		m_simulator->get_events ()->wait ();
	}
}
