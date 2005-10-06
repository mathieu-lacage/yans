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

	/* manage a new fiber. */
	void register_new_fiber (Fiber *fiber);

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
class TestFiberScheduler : public Test {
public:
	TestFiberScheduler (TestManager *manager);
	virtual ~TestFiberScheduler ();

	virtual bool run_tests (void);

	void record_run (uint8_t n);
private:
	uint8_t get_run (uint8_t n);
	uint8_t m_n;
};
#endif /* RUN_SELF_TESTS */

#endif /* FIBER_SCHEDULER_H */
