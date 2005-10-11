/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "runnable.h"

class Fiber;
class Semaphore;
class SleepEvent;

class Thread : public Runnable {
public:
	Thread (char const *name);
	Thread (Host *host, char const *name);
	virtual ~Thread ();

	void sleep_s (double delta);
	void sleep_us (uint64_t delta);

	/* blocking wait for notify. */
	void wait (void);
	/* unblock any blocked wait. */
	void notify (void);

	//static Thread *current (void);
private:
	virtual void run (void) = 0;
	
	Fiber *m_fiber;
	Semaphore *m_sleep_sem;
	SleepEvent *m_sleep;
};

#endif /* THREAD_H */


