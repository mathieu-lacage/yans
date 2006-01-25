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

#include "fiber-scheduler.h"
#include "fiber.h"
#include "fiber-context.h"
#include "simulator.h"
#include "event.tcc"
#include <cassert>

#define noSCHED_DEBUG 1


#ifdef SCHED_DEBUG
#  include <iostream>

static void
print_fiber (yans::Fiber const *f)
{
	std::cout << "\"" << *(f->peek_name ()) << "\" ";
	if (f->is_blocked ()) {
		std::cout << "blocked ";
	} else if (f->is_dead ()) {
		std::cout << "dead ";
	} else if (f->is_active ()) {
		std::cout << "active ";
	}
}
#  define TRACE_LEAVE(f) \
std::cout << "SCHED leave "; \
print_fiber (f);
#  define TRACE_ENTER_MAIN() \
std::cout << "enter main\n";
#  define TRACE_LEAVE_MAIN() \
std::cout << "SCHED leave main ";
#  define TRACE_ENTER(f) \
std::cout << "enter ";   \
print_fiber (f);         \
std::cout << std::endl;
#else
#  define TRACE_LEAVE(f)
#  define TRACE_ENTER_MAIN()
#  define TRACE_LEAVE_MAIN()
#  define TRACE_ENTER(f)
#endif

namespace yans {


FiberScheduler *FiberScheduler::m_instance = 0;

FiberScheduler::FiberScheduler ()
	: m_current (0)
{
	m_context = fiber_context_new_blank ();
	Simulator::insert_later (make_event (&FiberScheduler::schedule, this));
}

FiberScheduler::~FiberScheduler ()
{
	fiber_context_delete (m_context);
	m_context = (struct FiberContext *)0xdeadbeaf;
}
	
void 
FiberScheduler::schedule (void)
{
	Fiber *next, *prev;

	if (m_current != 0) {
		if (m_current->is_dead ()) {
			m_dead.push_back (m_current);
		} else if (m_current->is_blocked ()) {
			m_blocked.push_back (m_current);
		} else if (m_current->is_running ()) {
			m_current->set_active ();
			m_active.push_back (m_current);
		} else if (m_current->is_active ()) {
			m_active.push_back (m_current);
		}
		TRACE_LEAVE (m_current);
		prev = m_current;
		m_current = 0;

		next = peek_next_fiber ();
		if (next != 0) {
			Simulator::insert_in_s (40e-3, make_event (&FiberScheduler::schedule, this));
		}
		TRACE_ENTER_MAIN ();
		prev->switch_to (m_context);
	} else {
		TRACE_LEAVE_MAIN ();
		next = remove_next_fiber ();
		if (next != 0) {
			m_current = next;
			TRACE_ENTER (m_current);
			next->switch_from (m_context);
		} else {
			TRACE_ENTER_MAIN ();
		}		
	}

}

Fiber *
FiberScheduler::peek_next_fiber (void)
{
	if (!m_active.empty ()) {
		Fiber *fiber = m_active.front ();
		return fiber;
	}
	return 0;
}

Fiber *
FiberScheduler::remove_next_fiber (void)
{
	if (!m_active.empty ()) {
		Fiber *fiber = m_active.front ();
		m_active.pop_front ();
		return fiber;
	}
	return 0;
}

bool
FiberScheduler::is_all_fibers_dead (void)
{
	if (m_active.empty () &&
	    m_blocked.empty ()) {
		return true;
	} else {
		return false;
	}
}

bool
FiberScheduler::is_deadlock (void)
{
	if (!is_all_fibers_dead () &&
	    m_active.empty () &&
	    !m_blocked.empty ()) {
		return true;
	} else {
		return false;
	}
}


void
FiberScheduler::set_current_blocked (void)
{
	assert (m_current != 0);
	assert (m_current->is_running ());
	m_current->set_blocked ();
}

void 
FiberScheduler::set_active (Fiber *fiber)
{
	assert (fiber->is_blocked ());
	m_blocked.remove (fiber);
	fiber->set_active ();
	m_active.push_back (fiber);
}


void
FiberScheduler::register_fiber (Fiber *fiber)
{
	assert (fiber->is_active ());
	m_active.push_back (fiber);
}

void
FiberScheduler::unregister_fiber (Fiber *fiber)
{
	assert (fiber->is_dead ());
	m_dead.remove (fiber);
	if (m_current == 0 &&
	    m_active.empty () &&
	    m_dead.empty () &&
	    m_blocked.empty ()) {
		FiberScheduler::destroy ();
	}
}

Fiber *
FiberScheduler::get_current (void)
{
	return m_current;
}

void
FiberScheduler::destroy (void)
{
	delete m_instance;
	m_instance = 0;
}
FiberScheduler *
FiberScheduler::instance (void)
{
	if (m_instance == 0) {
		m_instance = new FiberScheduler ();
	}
	return m_instance;
}

}; // namespace yans


