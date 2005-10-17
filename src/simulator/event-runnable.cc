/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "event-runnable.h"
#include "clock.h"
#include "event-heap.h"
#include "event.h"
#include "simulator.h"
#include "fiber-scheduler.h"
#include "semaphore.h"
#include <math.h>

EventRunnable::EventRunnable ()
{
	m_stop = false;
	m_clock = new Clock ();
	m_event_heap = new EventHeap ();
	m_wait = new Semaphore (0);
}
EventRunnable::~EventRunnable ()
{
	delete m_clock;
	delete m_event_heap;
	delete m_wait;
}
void 
EventRunnable::insert_at_us (Event *event, uint64_t time)
{
	m_event_heap->insert_at_us (event, time);
	m_wait->up ();
}
void 
EventRunnable::insert_at_s (Event *event, double time)
{
	assert (time > 0);
	long int us = lrint (time * 1000000);
	assert (us > 0);
	m_event_heap->insert_at_us (event, (uint64_t)us);
	m_wait->up ();
}
void 
EventRunnable::insert_in_us (Event *event, uint64_t delta)
{
	uint64_t current = m_clock->get_current_us ();
	m_event_heap->insert_at_us (event, current+delta);
	m_wait->up ();
}
void 
EventRunnable::insert_in_s (Event *event, double delta)
{
	double current = m_clock->get_current_s ();
	insert_at_s (event, current+delta);
}
uint64_t 
EventRunnable::now_us (void)
{
	return m_clock->get_current_us ();
}
double 
EventRunnable::now_s (void)
{
	return m_clock->get_current_s ();
}


void
EventRunnable::stop (void)
{
	m_stop = true;
	m_wait->up ();
}
void 
EventRunnable::run (void)
{
	Event *ev = m_event_heap->peek_next ();
	uint64_t now = m_event_heap->peek_next_time_us ();
	while (!m_stop) {
		while (ev != 0 && !m_stop) {
			m_clock->update_current_us (now);
			ev->notify ();
			ev = m_event_heap->peek_next ();
			now = m_event_heap->peek_next_time_us ();
			FiberScheduler::instance ()->schedule ();
		}
		m_wait->down ();
		m_wait->up ();
	}
}