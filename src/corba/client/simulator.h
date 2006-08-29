/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#ifndef YAPNS_SIMULATOR_H
#define YAPNS_SIMULATOR_H

#include <stdint.h>
#include "event.h"

namespace {
class SimulatorPrivate;
};

namespace yapns {

class SimulationContextFactory;

class Simulator {
public:
	static void run (void);

	static void insert_in_s (double delta, Event *ev);
	static void insert_in_us (uint64_t delta_us, Event *ev);

	static void destroy (void);

	static void record_context_factory (SimulationContextFactory *ctx_factory);
private:
	static SimulatorPrivate *get_priv (void);
	static SimulatorPrivate *m_priv;
};

}; // namespace yapns


#endif /* YAPNS_SIMULATOR_H */
