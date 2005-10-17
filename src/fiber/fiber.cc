/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber.h"
#include "fiber-scheduler.h"
#include <cassert>
#include <string.h>
#include "fiber-context.h"
#include "fiber-stack.h"
#include "runnable.h"
#include "semaphore.h"

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
	m_context = fiber_context_new (Fiber::run_static, this, stack_size);
	FiberScheduler::instance ()->register_fiber (this);
	m_sem_dead = new Semaphore (0);
}
Fiber::~Fiber ()
{
	assert (m_state == DEAD);
	FiberScheduler::instance ()->unregister_fiber (this);
	fiber_context_delete (m_context);
	m_context = (FiberContext *)0xdeadbeaf;
	delete m_sem_dead;
}

void 
Fiber::run_static (void *data)
{
	Fiber *fiber = reinterpret_cast<Fiber *> (data);
	fiber->run ();
}

void
Fiber::run (void)
{
	m_runnable->run ();
	set_dead ();
	FiberScheduler::instance ()->schedule ();
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
Fiber::switch_from (struct FiberContext *from)
{
	m_state = RUNNING;
	fiber_context_switch_to (from, m_context);
}
void 
Fiber::switch_to (struct FiberContext *to)
{
	assert (m_state != RUNNING);
	fiber_context_switch_to (m_context, to);
}

void 
Fiber::switch_to (Fiber *to)
{
	assert (m_state != RUNNING);
	to->m_state = RUNNING;
	fiber_context_switch_to (m_context, to->m_context);
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

