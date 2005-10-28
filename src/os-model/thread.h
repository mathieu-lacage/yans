/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

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

	void yield (void);

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


