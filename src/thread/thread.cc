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
#include "event.tcc"
#include "semaphore.h"
#include "fiber.h"
#include "simulator.h"
#include "fiber-scheduler.h"

#define noTRACE_THREAD 1

#ifdef TRACE_THREAD
#include <iostream>
# define TRACE(x) \
std::cout << "THREAD TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_THREAD */
# define TRACE(format,...)
#endif /* TRACE_THREAD */


namespace yans {

Thread::Thread (char const *name)
{
	m_sleep_sem = new Semaphore (0);
	m_fiber = new Fiber (make_callback (&Thread::run, this), name);
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
	TRACE ("sleep for " << delta << "s");
	Simulator::insert_in_s (delta, make_event (&Semaphore::up, m_sleep_sem));
	m_sleep_sem->down ();
}
void 
Thread::sleep_us (uint64_t delta)
{
	TRACE ("sleep for " << delta << "us");
	Simulator::insert_in_us (delta, make_event (&Semaphore::up, m_sleep_sem));
	m_sleep_sem->down ();
}

double 
Thread::time_s (void)
{
	return Simulator::now_s ();
}

}; // namespace yans


#ifdef RUN_SELF_TESTS

#include <iostream>

namespace yans {

class A : public Thread {
public:
	A () : Thread ("A") {}
private:
	void run (void) {
		TRACE ("run");
		sleep_s (1.0);
		TRACE ("run completed");
	}
};


class B : public yans::Thread {
public:
	B () : yans::Thread ("B"), 
	       m_sem (new yans::Semaphore (0)),
	       m_osem (new yans::Semaphore (0)),
	       m_oosem (new yans::Semaphore (0)),
	       m_is_bad (false)
	{}
	~B () {delete m_sem; delete m_osem; delete m_oosem;}

	void wait_until_notify (void) {
		m_sem->down ();
		register double test_float = 200.0;
		m_osem->up ();
		m_oosem->down ();
		if (test_float != 200.0) {
			TRACE ("Float problem !!");
			m_is_bad = true;
		}
	}
	bool is_bad (void) {
		return m_is_bad;
	}
private:
	virtual void run (void) {
		TRACE ("B run");
		register double test_float = 100.0;
		sleep_s (10.0);
		m_sem->up ();
		m_osem->down ();
		if (test_float != 100.0) {
			m_is_bad = true;
			TRACE ("Float problem !!");
		}
		m_oosem->up ();
		TRACE ("B run completed");
	}
	yans::Semaphore *m_sem;
	yans::Semaphore *m_osem;
	yans::Semaphore *m_oosem;
	bool m_is_bad;
};

class C : public yans::Thread {
public:
  C (B *b) : yans::Thread ("C"), m_b (b) {}
private:
  virtual void run (void) {
    TRACE ("C run");
    m_b->wait_until_notify ();
    TRACE ("C completed");
  }
  B* m_b;
};


ThreadTest::ThreadTest ()
{}
bool 
ThreadTest::run_tests (void)
{
	bool ok = true;

	Thread *a = new A ();

	TRACE ("a created");

	Simulator::run ();

	TRACE ("simulation completed");

	delete a;

	Simulator::destroy ();

	B *b = new B ();
	C *c = new C (b);

	yans::Simulator::run ();

	yans::Simulator::destroy ();

	if (b->is_bad ()) {
		ok = false;
	}

	delete b;
	delete c;

	return ok;
}

}; // namespace yans

#endif /* RUN_SELF_TESTS */
