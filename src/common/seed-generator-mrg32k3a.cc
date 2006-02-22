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
#include "seed-generator.h"
#include "random-uniform.h"
#include "simulator.h"
#include "event.tcc"
#include "rng-mrg32k3a.h"

namespace yans {

class SeedGeneratorPrivate {
public:
	SeedGeneratorPrivate ();
	~SeedGeneratorPrivate ();
	void destroy (void);
	void reset (uint32_t seed);
	uint32_t get (void);
private:
	RngMrg32k3a m_generator;
};

SeedGeneratorPrivate::SeedGeneratorPrivate ()
{
	m_generator.reset (0);
}
SeedGeneratorPrivate::~SeedGeneratorPrivate ()
{}
void 
SeedGeneratorPrivate::destroy (void)
{
	delete this;
}
void 
SeedGeneratorPrivate::reset (uint32_t seed)
{
	m_generator.reset (seed);
}
uint32_t 
SeedGeneratorPrivate::get (void)
{
	return m_generator.get_uint ();
}



void 
SeedGenerator::reset (uint32_t seed)
{
	get_priv ()->reset (seed);
}
uint32_t 
SeedGenerator::get (void)
{
	return get_priv ()->get ();
}

SeedGeneratorPrivate *
SeedGenerator::get_priv (void)
{
	static SeedGeneratorPrivate *self = 0;
	if (self == 0) {
		self = new SeedGeneratorPrivate ();
		Simulator::insert_at_destroy (make_event (&SeedGeneratorPrivate::destroy, self));
	}
	return self;
}


}; // namespace yans
