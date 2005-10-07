/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef EVENT_FIBER_H
#define EVENT_FIBER_H

#include "fiber.h"

class Simulator;

class EventFiber : public Fiber {
public:
	EventFiber ();

	void set_simulator (Simulator *simulator);

	void stop (void);
private:
	virtual void run (void);
	Simulator *m_simulator;
	bool m_stop;
};

#endif /* EVENT_FIBER_H */
