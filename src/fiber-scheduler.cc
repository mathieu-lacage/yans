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
	TestFiber (TestFiberScheduler *test, uint8_t max)
		: m_test (test), m_max (max) {}
private:
	virtual void run (void) {
		for (uint8_t i = 0; i < m_max; i++) {
			m_test->record_run (this);
			FiberScheduler::instance ()->schedule ();
		}
		FiberScheduler::instance ()->schedule ();
	}
	TestFiberScheduler *m_test;
	uint8_t m_max;
};

TestFiberScheduler::TestFiberScheduler (TestManager *manager)
	: Test (manager)
{}
TestFiberScheduler::~TestFiberScheduler ()
{}

void 
TestFiberScheduler::record_run (Fiber const *fiber)
{
	if (m_runs.find (fiber) == m_runs.end ()) {
		m_runs[fiber] = 0;
	} else {
		m_runs[fiber] ++;
	}
}
uint32_t 
TestFiberScheduler::get_run (Fiber const *fiber)
{
	return m_runs[fiber];
	return 0;
}

bool
TestFiberScheduler::ensure_dead (Fiber const *fiber)
{
	bool ok = true;
	if (!fiber->is_dead ()) {
		ok = false;
		failure () << "FiberScheduler -- "
			   << "expected dead fiber."
			   << std::endl;
	}
	return ok;
}

bool
TestFiberScheduler::ensure_runs (Fiber const *fiber, uint32_t expected)
{
	bool ok;
	if (get_run (fiber) != 5) {
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
	TestFiber *fiber = new TestFiber (this, 5);
	sched->run_main ();
	ENSURE_DEAD (fiber);
	delete fiber;
	TestFiber *fiber1 = new TestFiber (this, 9);
	TestFiber *fiber2 = new TestFiber (this, 7);
	sched->run_main ();
	ENSURE_RUNS (fiber1, 9);
	ENSURE_DEAD (fiber1);
	ENSURE_RUNS (fiber2, 7);
	ENSURE_DEAD (fiber2);
	
	return ok;
}

#endif /* RUN_SELF_TESTS */
