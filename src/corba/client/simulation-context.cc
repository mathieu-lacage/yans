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
#include "start-remote-contexts.h"
#include "registry.h"
#include "registry_impl.h"
#include "yans/callback.h"

namespace yapns {

SimulationContextFactory::SimulationContextFactory ()
	: m_started (false)
{}
void 
SimulationContextFactory::initialize (int argc, char *argv[])
{
	m_orb = CORBA::ORB_init (argc, argv, "mico-local-orb");
}

void
SimulationContextFactory::started_cb (void)
{
	m_started = true;
}

void 
SimulationContextFactory::read_configuration (char const *filename)
{
	CORBA::Object_var poa_obj = m_orb->resolve_initial_references ("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
	PortableServer::POAManager_var manager = poa->the_POAManager ();

	Registry_impl *servant = new Registry_impl (m_orb);

	PortableServer::ObjectId_var object_id = poa->activate_object (servant);
 
	manager->activate ();

	Registry_var registry = servant->_this ();
	CORBA::String_var ref = m_orb->object_to_string (registry);
	StartRemoteContexts *remote_contexts = new StartRemoteContexts (ref, registry, "filename.xml");
	servant->set_callback (yans::make_callback (&StartRemoteContexts::registered, remote_contexts));

	remote_contexts->set_started_callback (yans::make_callback (&SimulationContextFactory::started_cb, this));

	while (!m_started) {
		if (m_orb->work_pending ()) {
			m_orb->perform_work ();
		}
	}
	// we are done starting the remote contexts !
	m_a = new SimulationContextImpl (registry->lookup ("a"));
	m_b = new SimulationContextImpl (registry->lookup ("b"));
}
SimulationContext 
SimulationContextFactory::lookup (std::string name)
{
	if (name.compare ("a") == 0) {
		return m_a;
	} else if (name.compare ("b") == 0) {
		return m_b;
	} else {
		assert (false);
		return 0;
	}
}



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
