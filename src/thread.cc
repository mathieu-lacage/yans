/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "thread.h"
#include "event.h"
#include "semaphore.h"
#include "fiber.h"
#include "simulator.h"


class SleepEvent : public Event {
public:
	SleepEvent (Semaphore *sem);
	virtual ~SleepEvent ();

	virtual void notify (void);
	virtual void notify_canceled (void);
private:
	Semaphore *m_sem;
};

SleepEvent::SleepEvent (Semaphore *sem) 
	: m_sem (sem)
{}

SleepEvent::~SleepEvent () 
{}

void 
SleepEvent::notify (void)
{
	m_sem->up ();
}
void 
SleepEvent::notify_canceled (void)
{
	m_sem->up ();
}

Thread::Thread (char const *name)
{
	m_sleep_sem = new Semaphore (0);
	m_sleep = new SleepEvent (m_sleep_sem);
	m_fiber = new Fiber (this, name);
}

Thread::~Thread ()
{
	delete m_sleep_sem;
	delete m_sleep;
}

void 
Thread::sleep_s (double delta)
{
	Simulator::instance ()->insert_in_s (m_sleep, delta);
	m_sleep_sem->down ();
}
void 
Thread::sleep_us (uint64_t delta)
{
	Simulator::instance ()->insert_in_us (m_sleep, delta);
	m_sleep_sem->down ();
}

void 
Thread::wait (void)
{}
void 
Thread::notify (void)
{}

