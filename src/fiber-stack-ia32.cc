/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber-stack.h"

#include <setjmp.h>



FiberStack::FiberStack (uint32_t stack_size)
{
	m_stack = new uint8_t [stack_size];
	m_stack_size = stack_size;
	memset (m_stack, 0, m_stack_size);
}
FiberStack::~FiberStack ()
{
	memset (m_stack, 0x66, m_stack_size);
	delete m_stack;
	m_stack = (uint8_t *)0xdeadbeaf;
	m_stack_size = 0xdeadbeaf;
}

void
FiberStack::static_run (FiberStack *fiber_stack)
{
	fiber_stack->run ();
}

void
FiberStack::run_on_new_stack (void)
{
	register void *obj = (void*) this;
	__asm__ ("movl %%ebp, %%ecx \n\t"
		 "movl %0, %%ebp    \n\t"
		 "movl %0, %%esp    \n\t"
		 "pushl %%ecx       \n\t"
		 "pushl %1          \n\t"
		 "call *%2          \n\t"
		 "addl $4, %%esp    \n\t"
		 "popl %%ebp        \n\t"
		 "leave             \n\t"
		 "ret               \n\t"
		 : 
		 : "r" (m_stack+m_stack_size-4),
		   "r" (obj),
		   "r" (FiberStack::static_run)
		 : "%ecx");
}


#ifdef RUN_SELF_TESTS
#include "test.h"

class MyFiberStack : public FiberStack {
public:
	MyFiberStack () 
		: FiberStack (4096),
		  m_has_run (false)
	{}
	virtual ~MyFiberStack ()
	{}

	bool has_run (void) {
		return m_has_run;
	}
private:
	virtual void run (void) {
		m_has_run = true;
	}
	bool m_has_run;
};

FiberStackTest::FiberStackTest (TestManager *manager)
	: Test (manager)
{}

FiberStackTest::~FiberStackTest ()
{}

bool
FiberStackTest::run_tests (void)
{
	bool ok = true;
	MyFiberStack *stack = new MyFiberStack ();
	if (stack->has_run ()) {
		ok = false;
		failure () << "FiberStack -- failure 1";
		failure () << std::endl;
	}
	stack->run_on_new_stack ();
	if (!stack->has_run ()) {
		ok = false;
		failure () << "FiberStack -- failure 2";
		failure () << std::endl;
	}
	return ok;
}

#endif /* RUN_SELF_TESTS */
