/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_SCHEDULER_H
#define FIBER_SCHEDULER_H

#include <list>
#include "fiber-context.h"

class Fiber;

class FiberScheduler {
public:
	FiberScheduler ();
	
	void schedule (void);

	/* returns when the last fiber is dead. */
	void run_main (void);
	void run_main_one (void);
	bool is_work_left (void);

	void set_active (Fiber *fiber);
	void set_current_blocked (void);

	/* manage a new fiber. */
	void register_fiber (Fiber *fiber);
	void unregister_fiber (Fiber *fiber);

	Fiber *get_current (void);

	static FiberScheduler *instance (void);
private:
	void update_current_state (void);
	void update_next_state (void);
	bool is_all_fibers_dead (void);
	bool is_deadlock (void);
	Fiber *select_next_fiber (void);

	typedef std::list<Fiber *> Fibers;
	static FiberScheduler *m_instance;
	FiberContext m_context;
	Fiber *m_current;
	Fibers m_active;
	Fibers m_dead;
	Fibers m_blocked;
	int m_do_n;
};

#ifdef RUN_SELF_TESTS
#include "test.h"
#include <map>
class TestRunnable;
class TestFiberScheduler : public Test {
public:
	TestFiberScheduler (TestManager *manager);
	virtual ~TestFiberScheduler ();

	virtual bool run_tests (void);

	void record_run (TestRunnable const *fiber);
private:
	bool ensure_dead (TestRunnable const *fiber);
	bool ensure_runs (TestRunnable const *fiber, uint32_t expected);
	uint32_t get_run (TestRunnable const *fiber);
	void clear_runs (void);
	typedef std::map<TestRunnable const*, uint32_t> Runs;
	typedef std::map<TestRunnable const*, uint32_t>::iterator RunsI;
	Runs m_runs;
};
#endif /* RUN_SELF_TESTS */

#endif /* FIBER_SCHEDULER_H */
