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

#define noSCHED_DEBUG 1

#ifdef SCHED_DEBUG
#  include <stdio.h>
static void
print_fiber (Fiber const *f)
{
	printf ("\"%s\" ", f->peek_name ()->c_str ());
	if (f->is_blocked ()) {
		printf ("blocked ");
	} else if (f->is_dead ()) {
		printf ("dead ");
	} else if (f->is_active ()) {
		printf ("active ");
	}
}
#  define TRACE_LEAVE(f) \
printf ("SCHED leave "); \
print_fiber (f);
#  define TRACE_ENTER_MAIN() \
printf ("enter main\n")
#  define TRACE_LEAVE_MAIN() \
printf ("SCHED leave main ")
#  define TRACE_ENTER(f) \
printf ("enter ");       \
print_fiber (f);         \
printf ("\n");
#else
#  define TRACE_LEAVE(f)
#  define TRACE_ENTER_MAIN()
#  define TRACE_LEAVE_MAIN()
#  define TRACE_ENTER(f)
#endif


FiberScheduler *FiberScheduler::m_instance = 0;

FiberScheduler::FiberScheduler ()
	: m_current (0)
{
	m_context = fiber_context_new_blank ();
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

		if (m_do_n > 0) {
			m_do_n--;
		}
		next = select_next_fiber ();
		if (next != 0 && m_do_n != 0) {
			m_current = next;
			TRACE_ENTER (m_current);
			prev->switch_to (next);
		} else {
			TRACE_ENTER_MAIN ();
			prev->switch_to (m_context);
		}
	} else {
		TRACE_LEAVE_MAIN ();
		if (m_do_n > 0) {
			m_do_n--;
		}
		next = select_next_fiber ();
		if (next != 0 && m_do_n != 0) {
			m_current = next;
			TRACE_ENTER (m_current);
			next->switch_from (m_context);
		} else {
			TRACE_ENTER_MAIN ();
		}		
	}

}

Fiber *
FiberScheduler::select_next_fiber (void)
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
FiberScheduler::run_main (void)
{
	m_do_n = -1;
	while (!is_all_fibers_dead () &&
	       !is_deadlock ()) {
		schedule ();
	}
}

void 
FiberScheduler::run_main_one (void)
{
	m_do_n = 1;
	assert (m_current == 0);
	schedule ();
}
bool 
FiberScheduler::is_work_left (void)
{
	if (!is_all_fibers_dead () &&
	    !is_deadlock ()) {
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
}

Fiber *
FiberScheduler::get_current (void)
{
	return m_current;
}

void
FiberScheduler::destroy (void)
{
	delete this;
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


#ifdef RUN_SELF_TESTS

#include "runnable.h"

class TestRunnable : public Runnable {
public:
	TestRunnable (char const *name, TestFiberScheduler *test, uint8_t max)
		: m_fiber (new Fiber (0, this, name)), 
		  m_test (test), 
		  m_max (max) {}
	virtual ~TestRunnable () {
		delete m_fiber;
		m_fiber = (Fiber *)0xdeadbeaf;
		m_max = 0x66;
		m_test = (TestFiberScheduler *)0xdeadbeaf;
	}

	bool is_dead (void) const {
		return m_fiber->is_dead ();
	}
private:
	virtual void run (void) {
		for (uint8_t i = 0; i < m_max; i++) {
			m_test->record_run (this);
			FiberScheduler::instance ()->schedule ();
		}
	}
	Fiber *m_fiber;
	TestFiberScheduler *m_test;
	uint8_t m_max;
};

TestFiberScheduler::TestFiberScheduler (TestManager *manager)
	: Test (manager)
{}
TestFiberScheduler::~TestFiberScheduler ()
{}

void 
TestFiberScheduler::record_run (TestRunnable const *fiber)
{
	if (m_runs.find (fiber) == m_runs.end ()) {
		m_runs[fiber] = 1;
	} else {
		m_runs[fiber] ++;
	}
}
uint32_t 
TestFiberScheduler::get_run (TestRunnable const *fiber)
{
	return m_runs[fiber];
}
void
TestFiberScheduler::clear_runs (void)
{
	m_runs.clear ();
}

bool
TestFiberScheduler::ensure_dead (TestRunnable const *runnable)
{
	bool ok = true;
	if (!runnable->is_dead ()) {
		ok = false;
		failure () << "FiberScheduler -- "
			   << "expected dead fiber."
			   << std::endl;
	}
	return ok;
}

bool
TestFiberScheduler::ensure_runs (TestRunnable const *fiber, uint32_t expected)
{
	bool ok = true;
	if (get_run (fiber) != expected) {
		ok = false;
		failure () << "FiberScheduler -- "
			   << "expected runs: " << expected << ", "
			   << "got: " << get_run (fiber)
			   << std::endl;
	}
	return ok;
}


#define ENSURE_DEAD(f)  \
if (!ensure_dead (f)) { \
	ok = false;     \
}
#define ENSURE_RUNS(f,expected)   \
if (!ensure_runs (f, expected)) { \
	ok = false;               \
}

bool 
TestFiberScheduler::run_tests (void)
{
	bool ok = true;
	FiberScheduler *sched = FiberScheduler::instance ();

	TestRunnable *fiber = new TestRunnable ("test0", this, 5);
	sched->run_main ();
	ENSURE_DEAD (fiber);
	clear_runs ();
	delete fiber;

	TestRunnable *fiber1 = new TestRunnable ("test1", this, 9);
	TestRunnable *fiber2 = new TestRunnable ("test2", this, 3);
	sched->run_main ();
	ENSURE_RUNS (fiber1, 9);
	ENSURE_DEAD (fiber1);
	ENSURE_RUNS (fiber2, 3);
	ENSURE_DEAD (fiber2);
	clear_runs ();
	ENSURE_DEAD (fiber1);
	ENSURE_DEAD (fiber2);
	delete fiber1;
	delete fiber2;

	return ok;
}

#endif /* RUN_SELF_TESTS */
