/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber.h"
#include "fiber-scheduler.h"
#include <cassert>
#include <string.h>
#include "fiber-context.h"
#include "fiber-stack.h"
#include "runnable.h"
#include "semaphore.h"

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
	bool m_first_run;
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
	printf ("switch from %p\n", this);
	m_context.load ();
	assert (false);
}
void 
FiberContextStack::save (void)
{
	printf ("save  in %p\n", this);
	m_context.save ();
}


void 
FiberContextStack::run (void)
{
	m_first_run = true;
	m_context.save ();
	if (m_first_run) {
		m_first_run = false;
		return;
	}
	printf ("go into fiber\n");
	m_fiber->run ();
	m_fiber->set_dead ();
	FiberScheduler::instance ()->schedule ();
}


uint32_t const Fiber::DEFAULT_STACK_SIZE = 8192;


Fiber::Fiber (Host *host, Runnable *runnable, char const *name)
	: m_host (host), m_runnable (runnable)
{
	initialize (name, DEFAULT_STACK_SIZE);
}
Fiber::Fiber (Host *host, Runnable *runnable, char const *name, uint32_t stack_size)
	: m_host (host), m_runnable (runnable)
{
	initialize (name, stack_size);
}

Fiber::Fiber (Runnable *runnable, char const *name)
	: m_runnable (runnable)
{
	m_host = FiberScheduler::instance ()->get_current ()->get_host ();
	initialize (name, DEFAULT_STACK_SIZE);
}
Fiber::Fiber (Runnable *runnable, char const *name, uint32_t stack_size)
	: m_runnable (runnable)
{
	m_host = FiberScheduler::instance ()->get_current ()->get_host ();
	initialize (name, stack_size);
}
void
Fiber::initialize (char const *name, uint32_t stack_size)
{
	m_name = new std::string (name);
	m_state = ACTIVE;
	m_stack = new FiberContextStack (this, stack_size);
	FiberScheduler::instance ()->register_fiber (this);
	m_sem_dead = new Semaphore (0);
	m_stack->run_on_new_stack ();
}
Fiber::~Fiber ()
{
	assert (m_state == DEAD);
	FiberScheduler::instance ()->unregister_fiber (this);
	delete m_stack;
	m_stack = (FiberContextStack *)0xdeadbeaf;
	delete m_sem_dead;
}

std::string *
Fiber::peek_name (void) const
{
	return m_name;
}

bool 
Fiber::is_running (void) const
{
	return (m_state == RUNNING)?true:false;
}
bool 
Fiber::is_active (void) const
{
	return (m_state == ACTIVE)?true:false;
}
bool 
Fiber::is_blocked (void) const
{
	return (m_state == BLOCKED)?true:false;
}
bool 
Fiber::is_dead (void) const
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
	m_sem_dead->up_all ();
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
	assert (false);
}

Host *
Fiber::get_host (void) const
{
	return m_host;
}

void 
Fiber::wait_until_is_dead (void)
{
	m_sem_dead->down ();
}

void
Fiber::run (void)
{
	m_runnable->run ();
}
