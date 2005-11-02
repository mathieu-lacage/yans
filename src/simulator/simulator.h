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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "block.h"
#include <stdint.h>

class Event;
class Fiber;
class EventRunnable;
class EventHeap;
class Clock;

class Simulator {
public:
	void destroy (void);

	/* this is a blocking call which will return
	 * only when the simulation ends.
	 */
	void run (void) BLOCK;
	/* This is a non-blocking call which will 
	 * unblock the run method.
	 */
	void stop (void);

	/* in microseconds. */
	void insert_in_us (Event *event, uint64_t delta);
	void insert_at_us (Event *event, uint64_t time);
	uint64_t now_us (void);

	/* in seconds. */
	void insert_in_s (Event *event, double delta);
	void insert_at_s (Event *event, double time);
	double now_s (void);

	static Simulator *instance (void);
private:
	Simulator ();
	~Simulator ();

	static Simulator *m_instance;
	EventRunnable *m_event_runnable;
	Fiber *m_event_fiber;
};

#endif /* SIMULATOR_H */
