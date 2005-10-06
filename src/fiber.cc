/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber.h"
#include "fiber-scheduler.h"
#include <cassert>
#include <string.h>
#include "fiber-context.h"
#include "fiber-stack.h"

class FiberContextStack : public FiberStack {
public:
	FiberContextStack (Fiber *fiber, uint32_t stack_size);
	virtual ~FiberContextStack ();

	void switch_to (void);
	void save (void);

private:
	virtual void run (void);
	FiberContext m_context;
	Fiber *m_fiber;
};


FiberContextStack::FiberContextStack (Fiber *fiber, uint32_t stack_size)
	: FiberStack (stack_size)
{
	m_fiber = fiber;
}
FiberContextStack::~FiberContextStack ()
{
	m_fiber = (Fiber *)0xdeadbeaf;
}

void 
FiberContextStack::switch_to (void)
{
	m_context.load ();
}
void 
FiberContextStack::save (void)
{
	m_context.save ();
}


void 
FiberContextStack::run (void)
{
	bool first_run = true;
	m_context.save ();
	if (first_run) {
		return;
	}
	m_fiber->run ();
	m_fiber->set_dead ();
	FiberScheduler::instance ()->schedule ();
}



Fiber::Fiber ()
{
	m_stack = new FiberContextStack (this, 8192);
	FiberScheduler::instance ()->register_new_fiber (this);
	m_stack->run_on_new_stack ();
}
Fiber::Fiber (uint32_t stack_size)
{
	m_stack = new FiberContextStack (this, stack_size);
	FiberScheduler::instance ()->register_new_fiber (this);
	m_stack->run_on_new_stack ();
}
Fiber::~Fiber ()
{
	delete m_stack;
	m_stack = (FiberContextStack *)0xdeadbeaf;
}

bool 
Fiber::is_running (void)
{
	return (m_state == RUNNING)?true:false;
}
bool 
Fiber::is_active (void)
{
	return (m_state == ACTIVE)?true:false;
}
bool 
Fiber::is_blocked (void)
{
	return (m_state == BLOCKED)?true:false;
}
bool 
Fiber::is_dead (void)
{
	return (m_state == DEAD)?true:false;
}

void 
Fiber::set_blocked (void)
{
	m_state = BLOCKED;
}
void 
Fiber::set_active (void)
{
	m_state = ACTIVE;
}
void
Fiber::set_dead (void)
{
	m_state = DEAD;
}

void
Fiber::save (void)
{
	m_stack->save ();
}

void 
Fiber::switch_to (void)
{
	m_state = RUNNING;
	m_stack->switch_to ();
}
