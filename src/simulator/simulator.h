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

#include <stdint.h>

namespace yans {

class Event;
class SimulatorPrivate;
class ParallelSimulatorQueuePrivate;

class ParallelSimulatorQueue {
public:
	virtual ~ParallelSimulatorQueue () = 0;
	void insert_at_us (uint64_t at, Event *ev);
private:
	friend class Simulator;
	friend class ParallelSimulatorQueuePrivate;

	void set_priv (ParallelSimulatorQueuePrivate *priv);
	virtual void send_null_message (void) = 0;
	ParallelSimulatorQueuePrivate *m_priv;
protected:
	ParallelSimulatorQueue ();
};

class Simulator {
public:
	static void set_linked_list (void);
	static void set_binary_heap (void);
	static void set_std_map (void);

	static void enable_log_to (char const *filename);

	static void destroy (void);

	static void add_parallel_queue (ParallelSimulatorQueue *queue);

	/* this is a blocking call which will return
	 * only when the simulation ends or when stop is
	 * invoked. 
	 */
	static void run (void);
	/* This is a non-blocking call which will 
	 * unblock the run method. run can be re-invoked
	 * right after a stop to restart the main loop
	 * where it was stopped.
	 */
	static void stop (void);
	/* stop only at the scheduled time.
	 * stop_at_us (at, make_event (foo));
	 * is subtly different from:
	 * void function (void) {
	 *    Simulator::stop ();
	 *    foo ();
	 * }
	 * Simulator::insert_at_us (at, make_event (function));
	 * because the former will not allow _any_ event
	 * whose timestamp is equal to at_us to run, even
	 * any event inserted before the stop event was inserted.
	 */
	static void stop_at_us (uint64_t at, Event *event);

	static Event *insert_in_us (uint64_t delta, Event *event);
	static Event *insert_in_s (double delta, Event *event);

	static Event *insert_at_us (uint64_t time, Event *event);
	static Event *insert_at_s (double time, Event *event);

	static Event *remove (Event const*id);

	static uint64_t now_us (void);
	static double now_s (void);

	static void insert_later (Event *event);

	static void insert_at_destroy (Event *event);
private:
	Simulator ();
	~Simulator ();
	static SimulatorPrivate *get_priv (void);
	static SimulatorPrivate *m_priv;
	static enum ListType {
		LINKED_LIST,
		BINARY_HEAP,
		STD_MAP
	} m_list_type;
};

}; // namespace yans

#endif /* SIMULATOR_H */
