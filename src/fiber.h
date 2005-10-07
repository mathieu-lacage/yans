/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_H
#define FIBER_H

#include <stdint.h>

class FiberContextStack;

class Fiber {
public:
	Fiber ();
	Fiber (uint32_t stack_size);
	virtual ~Fiber ();

	bool is_running (void) const;
	bool is_active (void) const;
	bool is_blocked (void) const;
	bool is_dead (void) const;

	void set_active (void);
	void set_blocked (void);

	void switch_to (void);

	void save (void);

private:
	friend class FiberContextStack;

	/* for children to override. */
	virtual void run (void) = 0;
	void set_dead (void);
	enum FiberState_e { 
		RUNNING,
		ACTIVE,
		BLOCKED,
		DEAD
	};
	enum FiberState_e m_state;
	FiberContextStack *m_stack;
};


#endif /* FIBER_H */
