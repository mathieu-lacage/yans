/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

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
