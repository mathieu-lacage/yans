/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_H
#define FIBER_H

#include <stdint.h>
#include <string>

class FiberContextStack;
class Host;
class Runnable;
class Semaphore;

class Fiber {
public:
	Fiber (Host *host, Runnable *runnable, char const *name);
	Fiber (Host *host, Runnable *runnable, char const *name, uint32_t stack_size);
	Fiber (Runnable *runnable, char const *name);
	Fiber (Runnable *runnable, char const *name, uint32_t stack_size);
	virtual ~Fiber ();

	bool is_running (void) const;
	bool is_active (void) const;
	bool is_blocked (void) const;
	bool is_dead (void) const;

	void switch_to (void);

	void save (void);

	std::string *peek_name (void) const;

	Host *get_host (void) const;

	void wait_until_is_dead (void);

private:
	static uint32_t const DEFAULT_STACK_SIZE;

	friend class FiberContextStack;
	friend class FiberScheduler;
	void initialize (char const *name, uint32_t stack_size);
	void set_dead (void);
	void set_active (void);
	void set_blocked (void);
	void run (void);

	enum FiberState_e { 
		RUNNING,
		ACTIVE,
		BLOCKED,
		DEAD
	};
	enum FiberState_e m_state;
	FiberContextStack *m_stack;
	std::string *m_name;
	Host *m_host;
	Runnable *m_runnable;
	Semaphore *m_sem_dead;
};


#endif /* FIBER_H */
