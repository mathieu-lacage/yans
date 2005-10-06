/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber-scheduler.h"
#include "fiber.h"
#include "fiber-context.h"

FiberScheduler *FiberScheduler::m_instance = 0;

FiberScheduler::FiberScheduler ()
	: m_current (0)
{}

void
FiberScheduler::update_current_state (void)
{
	if (m_current == 0) {
		return;
	}
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
	m_current->save ();
	if (m_current->is_running ()) {
		return;
	}
	m_current = 0;
}

void
FiberScheduler::update_next_state (void)
{
	if (m_do_n > 0) {
		m_do_n--;
	} else if (m_do_n == 0) {
		m_context.load ();
	}
	Fiber *new_fiber = select_next_fiber ();
	if (new_fiber == 0) {
		m_context.load ();
	} else {
		m_current = new_fiber;
		m_current->switch_to ();
	}
}
	
void 
FiberScheduler::schedule (void)
{
	update_current_state ();
	update_next_state ();
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
	m_context.save ();
	while (!is_all_fibers_dead () &&
	       !is_deadlock ()) {
		schedule ();
	}
}

void 
FiberScheduler::run_main_one (void)
{
	m_do_n = 1;
	m_context.save ();
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
FiberScheduler::register_new_fiber (Fiber *fiber)
{
	assert (fiber->is_active ());
	m_active.push_back (fiber);
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

class TestFiber : public Fiber {
public:
	TestFiber (uint8_t n, TestFiberScheduler *test)
		: m_n (n), m_test (test) {}
private:
	virtual void run (void) {
		for (uint8_t i = 0; i < 5; i++) {
			m_test->record_run (m_n);
			FiberScheduler::instance ()->schedule ();
		}
		set_blocked ();
		FiberScheduler::instance ()->schedule ();
	}
	uint8_t m_n;
	TestFiberScheduler *m_test;
};

TestFiberScheduler::TestFiberScheduler (TestManager *manager)
	: Test (manager)
{}
TestFiberScheduler::~TestFiberScheduler ()
{}

void 
TestFiberScheduler::record_run (uint8_t n)
{
	m_n++;
}
uint8_t 
TestFiberScheduler::get_run (uint8_t n)
{
	return m_n;
}

bool 
TestFiberScheduler::run_tests (void)
{
	bool ok = true;
	FiberScheduler *sched = FiberScheduler::instance ();
	new TestFiber (0x33, this);
	sched->run_main ();
	if (get_run (0x33) != 5) {
		ok = false;
		failure () << "FiberScheduler -- ";
		failure () << std::endl;
	}
	return ok;
}

#endif /* RUN_SELF_TESTS */
