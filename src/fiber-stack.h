/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_STACK_H
#define FIBER_STACK_H

#include <stdint.h>

class FiberStackPrivate;

class FiberStack {
public:
	FiberStack (uint32_t stack_size);
	virtual ~FiberStack ();

	/* This method returns in the old stack
	 * when the private run method has finished
	 * running on the new stack.
	 */
	void run_on_new_stack (void);

private:
	static void static_run (FiberStack *fiber_stack);
	virtual void run (void) = 0;
	uint8_t *m_stack;
	uint32_t m_stack_size;
};

#ifdef RUN_SELF_TESTS
#include "test.h"

class FiberStackTest : public Test {
public:
	FiberStackTest (TestManager *manager);
	virtual ~FiberStackTest ();
  
	virtual bool run_tests (void);
};
#endif /* RUN_SELF_TESTS */

#endif /* FIBER_STACK_H */
