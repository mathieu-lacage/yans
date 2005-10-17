/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_H
#define FIBER_H

#include <stdint.h>
#include <string>

struct FiberContext;
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

	std::string *peek_name (void) const;

	Host *get_host (void) const;

	void wait_until_is_dead (void);

private:
	friend class FiberScheduler;

	static uint32_t const DEFAULT_STACK_SIZE;

	void switch_from (struct FiberContext *from);
	void switch_to (struct FiberContext *to);
	void switch_to (Fiber *to);
	void initialize (char const *name, uint32_t stack_size);
	void set_dead (void);
	void set_active (void);
	void set_blocked (void);
	void run (void);
	static void run_static (void *data);

	enum FiberState_e { 
		RUNNING,
		ACTIVE,
		BLOCKED,
		DEAD
	};
	enum FiberState_e m_state;
	FiberContext *m_context;
	std::string *m_name;
	Host *m_host;
	Runnable *m_runnable;
	Semaphore *m_sem_dead;
};


#endif /* FIBER_H */
