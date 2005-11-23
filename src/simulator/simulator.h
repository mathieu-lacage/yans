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

class Event;
class SimulatorPrivate;

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
	static void insert_in_us (Event *event, uint64_t delta);
	static void insert_at_us (Event *event, uint64_t time);
	static uint64_t now_us (void);

	/* in seconds. */
	static void insert_in_s (Event *event, double delta);
	static void insert_at_s (Event *event, double time);
	static double now_s (void);
private:
	Simulator ();
	~Simulator ();
	static SimulatorPrivate *get_priv (void);
	static bool m_destroyed;
};

#endif /* SIMULATOR_H */
