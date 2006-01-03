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

#include "semaphore.h"
#include "fiber-scheduler.h"
#include "fiber.h"

#define TRACE_SEM 1

#ifdef TRACE_SEM
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "SEM TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_SEM */
# define TRACE(format,...)
#endif /* TRACE_SEM */

namespace yans {

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
	TRACE ("upping " << this << " by " << (uint16_t)delta);
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
	TRACE ("downing " << this << " by " << (uint16_t)delta);
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

}; // namespace yans
