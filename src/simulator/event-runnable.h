/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

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
