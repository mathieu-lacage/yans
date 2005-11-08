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

#include "simulator.h"
#include "clock.h"
#include "event-heap.h"
#include "tag-manager.h"

Simulator *Simulator::m_instance = 0;

class SimulatorPrivate {
public:
	SimulatorPrivate ();
	~SimulatorPrivate ();

	void run (void);
	void stop (void);
	void insert_in_us (Event *event, uint64_t delta);
	void insert_at_us (Event *event, uint64_t time);
	uint64_t now_us (void);
	void insert_in_s (Event *event, double delta);
	void insert_at_s (Event *event, double time);
	double now_s (void);

private:
};

SimulatorPrivate::SimulatorPrivate ()
{
}

SimulatorPrivate::~SimulatorPrivate ()
{
}

void
SimulatorPrivate::run (void)
{
}

void 
SimulatorPrivate::stop (void)
{
}
void 
SimulatorPrivate::insert_in_us (Event *event, uint64_t delta)
{
}
void 
SimulatorPrivate::insert_at_us (Event *event, uint64_t time)
{
}
uint64_t 
SimulatorPrivate::now_us (void)
{
	return 0;
}
void 
SimulatorPrivate::insert_in_s (Event *event, double delta)
{
}
void 
SimulatorPrivate::insert_at_s (Event *event, double time)
{
}
double 
SimulatorPrivate::now_s (void)
{
	return 0;
}


Simulator *
Simulator::instance (void)
{
	if (m_instance == 0) {
		m_instance = new Simulator ();
	}
	return m_instance;
}

void
Simulator::destroy (void)
{
	delete m_instance;
	m_instance = 0;
}

Simulator::Simulator ()
{
	m_priv = new SimulatorPrivate ();
}

Simulator::~Simulator ()
{
	delete m_priv;
	m_priv = (SimulatorPrivate *)0xdeadbeaf;
}

void 
Simulator::run (void)
{
	m_priv->run ();
}
void 
Simulator::stop (void)
{
	m_priv->stop ();
}
void 
Simulator::insert_in_us (Event *event, uint64_t delta)
{
	m_priv->insert_in_us (event, delta);
}
void 
Simulator::insert_at_us (Event *event, uint64_t time)
{
	m_priv->insert_at_us (event, time);
}
uint64_t 
Simulator::now_us (void)
{
	return m_priv->now_us ();
}
void 
Simulator::insert_in_s (Event *event, double delta)
{
	m_priv->insert_in_s (event, delta);
}
void 
Simulator::insert_at_s (Event *event, double time)
{
	m_priv->insert_at_s (event, time);
}
double 
Simulator::now_s (void)
{
	return m_priv->now_s ();
}

