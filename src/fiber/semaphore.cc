/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "semaphore.h"
#include "fiber-scheduler.h"
#include "fiber.h"

Semaphore::Semaphore (int32_t n)
	: m_n (n)
{}

Semaphore::~Semaphore ()
{
	assert (m_n >= 0);
	m_waiting.erase (m_waiting.begin (), m_waiting.end ());
	m_n = (int32_t) 0xdeadbeaf;
}

void 
Semaphore::up (void)
{
	up (1);
}
void 
Semaphore::down (void)
{
	down (1);
}

void 
Semaphore::up (uint8_t delta)
{
	m_n += delta;
	FiberScheduler *scheduler = FiberScheduler::instance ();
	if (m_n >= 0 && 
	    !m_waiting.empty ()) {
		Fiber *waiting = m_waiting.front ();
		assert (waiting->is_blocked ());
		m_waiting.pop_front ();
		scheduler->set_active  (waiting);
		scheduler->schedule ();
	}
}
void 
Semaphore::down (uint8_t delta)
{
	m_n -= delta;
	while (m_n < 0) {
		FiberScheduler *scheduler = FiberScheduler::instance ();
		Fiber *current = scheduler->get_current ();
		/* If the assert below is triggered, it means
		 * that you have tried using this semaphore from
		 * the scheduler mainloop which is _bad_. You must
		 * use this semaphore from a fiber managed by
		 * the scheduler.
		 */
		assert (current != 0);
		m_waiting.push_back (current);
		scheduler->set_current_blocked ();
		scheduler->schedule ();
	}
}

void
Semaphore::up_all (void)
{
	assert (m_n <= 0);
	up (-m_n);
}

void
Semaphore::down_all (void)
{
	assert (m_n >= 0);
	down (m_n);
}
