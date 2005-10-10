/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_H
#define FIBER_H

#include <stdint.h>
#include <string>

class FiberContextStack;
class Host;

class Fiber {
public:
	Fiber (Host *host, char const *name);
	Fiber (Host *host, char const *name, uint32_t stack_size);
	Fiber (char const *name);
	Fiber (char const *name, uint32_t stack_size);
	virtual ~Fiber ();

	bool is_running (void) const;
	bool is_active (void) const;
	bool is_blocked (void) const;
	bool is_dead (void) const;

	void set_active (void);
	void set_blocked (void);

	void switch_to (void);

	void save (void);

	std::string *peek_name (void) const;

	Host *get_host (void) const;

private:
	/* for children to override. */
	virtual void run (void) = 0;

	static uint32_t const DEFAULT_STACK_SIZE;

	friend class FiberContextStack;
	void initialize (char const *name, uint32_t stack_size);
	void set_dead (void);
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
};


#endif /* FIBER_H */
