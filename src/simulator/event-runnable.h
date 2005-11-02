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

#ifndef EVENT_RUNNABLE_H
#define EVENT_RUNNABLE_H

#include "runnable.h"
#include <stdint.h>

class Event;
class Simulator;
class Runnable;
class EventHeap;
class Clock;
class Semaphore;

class EventRunnable : public Runnable {
public:
	EventRunnable ();
	virtual ~EventRunnable ();

	void insert_at_us (Event *event, uint64_t time);
	void insert_at_s (Event *event, double time);
	void insert_in_us (Event *event, uint64_t delta);
	void insert_in_s (Event *event, double delta);
	uint64_t now_us (void);
	double now_s (void);

	void stop (void);
private:
	virtual void run (void);
	bool m_stop;
	EventHeap *m_event_heap;
	Clock *m_clock;
	Semaphore *m_wait;
};

#endif /* EVENT_RUNNABLE_H */
