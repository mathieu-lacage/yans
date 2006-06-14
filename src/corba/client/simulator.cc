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
#include "simulator.h"
#include "simulation-context.h"

namespace {
class SimulatorPrivate {
public:
	SimulatorPrivate ();
	~SimulatorPrivate ();
	void run (void);
	void insert_in_us (uint64_t us, yapns::Event *ev);
	void record_context_factory (yapns::SimulationContextFactory *ctx_factory);
private:
	yapns::SimulationContextFactory *m_ctx_factory;
};

SimulatorPrivate::SimulatorPrivate ()
{}
SimulatorPrivate::~SimulatorPrivate ()
{}
void 
SimulatorPrivate::run (void)
{
	m_ctx_factory->run ();
}
void 
SimulatorPrivate::insert_in_us (uint64_t us, yapns::Event *ev)
{
	m_ctx_factory->insert_at_us (m_ctx_factory->now_us () + us, ev);
}
void 
SimulatorPrivate::record_context_factory (yapns::SimulationContextFactory *ctx_factory)
{
	m_ctx_factory = ctx_factory;
}
};

namespace yapns {

SimulatorPrivate *Simulator::m_priv = 0;

SimulatorPrivate *
Simulator::get_priv (void)
{
	if (m_priv == 0) {
		m_priv = new SimulatorPrivate ();
	}
	return m_priv;
}

void 
Simulator::insert_in_us (uint64_t delta_us, Event *ev)
{
	get_priv ()->insert_in_us (delta_us, ev);
}

void 
Simulator::run (void)
{
	get_priv ()->run ();
}

void
Simulator::destroy (void)
{
	delete m_priv;
	m_priv = 0;
}

void 
Simulator::record_context_factory (SimulationContextFactory *ctx_factory)
{
	get_priv ()->record_context_factory (ctx_factory);
}


}; // namespace yapns
