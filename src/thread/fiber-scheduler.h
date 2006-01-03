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

#ifndef FIBER_SCHEDULER_H
#define FIBER_SCHEDULER_H

#include <list>
#include "fiber-context.h"

namespace yans {

class Fiber;

class FiberScheduler {
public:	
	void schedule (void);

	void set_active (Fiber *fiber);
	void set_current_blocked (void);

	/* manage a new fiber. */
	void register_fiber (Fiber *fiber);
	void unregister_fiber (Fiber *fiber);

	Fiber *get_current (void);

	void destroy (void);
	static FiberScheduler *instance (void);
private:
	FiberScheduler ();
	~FiberScheduler ();
	bool is_all_fibers_dead (void);
	bool is_deadlock (void);
	Fiber *select_next_fiber (void);

	typedef std::list<Fiber *> Fibers;
	static FiberScheduler *m_instance;
	FiberContext *m_context;
	Fiber *m_current;
	Fibers m_active;
	Fibers m_dead;
	Fibers m_blocked;
};

}; // namespace yans

#endif /* FIBER_SCHEDULER_H */
