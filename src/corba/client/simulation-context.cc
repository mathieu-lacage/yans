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
#include "simulation-context.h"

namespace yapns {

SimulationContextFactory::SimulationContextFactory ()
{}
void 
SimulationContextFactory::read_configuration (char const *filename)
{
	// XXX
	// open filename.xml and:
	//   - read remote and local context definitions.
	//   - start remote contexts
	//   - when a remote context is registered
	//     in the Registry, create its local contexts
	//   - when all local contexts are created, continue.
}
SimulationContext 
SimulationContextFactory::lookup (std::string name)
{}



SimulationContextImpl::SimulationContextImpl (::Remote::ComputingContext_ptr remote)
{
	m_context = ::Remote::ComputingContext::_duplicate (remote);
	// XXX ref remote.
}

SimulationContextImpl::~SimulationContextImpl ()
{
	CORBA::release (m_context);
	CORBA::release (m_80211_factory);
}

bool 
SimulationContextImpl::is_equal (SimulationContext o)
{
	return (o.get () == this);
}

::Remote::ComputingContext_ptr 
SimulationContextImpl::peek_remote (void) const
{
	return m_context;
}

::Remote::NetworkInterface80211Factory_ptr 
SimulationContextImpl::peek_80211_factory (void)
{
	if (CORBA::is_nil (m_80211_factory)) {
		m_80211_factory = m_context->create_network_interface_80211_factory ();
	}
	return m_80211_factory;
}


}; // namespace yapns
