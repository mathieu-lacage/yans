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
#ifndef YAPNS_SIMULATION_CONTEXT_H
#define YAPNS_SIMULATION_CONTEXT_H

#include <string>
#include "yans/reference-list.h"
#include "remote-context.h"

namespace yapns {

class SimulationContextImpl;

typedef yans::ReferenceList<SimulationContextImpl *> SimulationContext;

class SimulationContextFactory {
public:
	SimulationContextFactory ();
	void read_configuration (char const *filename);
	SimulationContext lookup (std::string name);
private:
};

class SimulationContextImpl {
public:
	SimulationContextImpl (::Remote::ComputingContext_ptr);
	::Remote::ComputingContext_ptr get_corba_context (void) const;
private:
	::Remote::ComputingContext_ptr m_context;
};

}; // namespace yapns

#endif /* YAPNS_SIMULATION_CONTEXT_H */
