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

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "block.h"
#include <stdint.h>

namespace yans {

class Event;
class SimulatorPrivate;

/* This simple macro is quite useful when 
 * debugging.
 */
#define STOP_AT(time)              \
if (Simulator::now_s () >= time) { \
	bool loop = true;          \
	while (loop) {}            \
}

class Simulator {
public:
	static void destroy (void);

	/* this is a blocking call which will return
	 * only when the simulation ends.
	 */
	static void run (void) BLOCK;
	/* This is a non-blocking call which will 
	 * unblock the run method.
	 */
	static void stop (void);

	/* in microseconds. */
	static void insert_in_us (uint64_t delta, Event *event);
	static void insert_at_us (uint64_t time, Event *event);
	static uint64_t now_us (void);

	/* in seconds. */
	static void insert_in_s (double delta, Event *event);
	static void insert_at_s (double time, Event *event);
	static double now_s (void);

	static void insert_later (Event *event);
private:
	Simulator ();
	~Simulator ();
	static SimulatorPrivate *get_priv (void);
	static bool m_destroyed;
};

}; // namespace yans

#endif /* SIMULATOR_H */
