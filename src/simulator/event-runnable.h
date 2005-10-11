/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef EVENT_RUNNABLE_H
#define EVENT_RUNNABLE_H

#include "runnable.h"

class Simulator;

class EventRunnable : public Runnable {
public:
	EventRunnable ();

	void set_simulator (Simulator *simulator);

	void stop (void);
private:
	virtual void run (void);
	Simulator *m_simulator;
	bool m_stop;
};

#endif /* EVENT_RUNNABLE_H */
