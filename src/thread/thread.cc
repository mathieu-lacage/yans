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

#include "thread.h"
#include "event.h"
#include "semaphore.h"
#include "fiber.h"
#include "simulator.h"
#include "fiber-scheduler.h"

namespace yans {

Thread::Thread (char const *name)
{
	m_sleep_sem = new Semaphore (0);
	m_fiber = new Fiber (this, name);
}

Thread::~Thread ()
{
	delete m_sleep_sem;
	delete m_fiber;
}

void 
Thread::yield (void)
{
	FiberScheduler::instance ()->schedule ();
}

void 
Thread::sleep_s (double delta)
{
	Simulator::insert_in_s (delta, make_event (&Thread::sleep_finished, this));
	m_sleep_sem->down ();
}
void 
Thread::sleep_us (uint64_t delta)
{
	Simulator::insert_in_us (delta, make_event (&Thread::sleep_finished, this));
	m_sleep_sem->down ();
}

void 
Thread::wait (void)
{}
void 
Thread::notify (void)
{}


void 
Thread::sleep_finished (void)
{
	m_sleep_sem->up ();
}

}; // namespace yans