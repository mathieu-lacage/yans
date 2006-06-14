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
#include "stopped-callback-impl.h"
#include "yans/callback.h"
#include "yans/scheduler-list.h"
#include "yans/event.h"
#include "yans/event.tcc"
#include "event.h"

namespace yapns {

SimulationContextFactory::SimulationContextFactory ()
	: m_started (false),
	  m_now_us (0),
	  m_uid (0),
	  m_scheduler (new yans::SchedulerList ())
{}
SimulationContextFactory::~SimulationContextFactory ()
{
	delete m_stopped_cb_servant;
	delete m_registry_servant;
	m_contexts.erase (m_contexts.begin (), m_contexts.end ());
}
void 
SimulationContextFactory::initialize (int argc, char *argv[])
{
	m_orb = CORBA::ORB_init (argc, argv, "mico-local-orb");
	CORBA::Object_var poa_obj = m_orb->resolve_initial_references ("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
	PortableServer::POAManager_var manager = poa->the_POAManager ();
	manager->activate ();
	m_stopped_cb_servant = new StoppedCallback_impl ();
	m_stopped_cb_servant->set_callback (yans::make_callback (&SimulationContextFactory::one_context_stopped, this));
	activate_servant (m_stopped_cb_servant);
	m_registry_servant = new Registry_impl (m_orb);
	activate_servant (m_registry_servant);

}

void
SimulationContextFactory::started_cb (void)
{
	m_started = true;
}

void
SimulationContextFactory::activate_servant (PortableServer::StaticImplementation *servant)
{
	CORBA::Object_var poa_obj = m_orb->resolve_initial_references ("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
	poa->activate_object (servant);
}


void 
SimulationContextFactory::read_configuration (char const *filename)
{
	Registry_var registry = m_registry_servant->_this ();
	CORBA::String_var ref = m_orb->object_to_string (registry);
	StartRemoteContexts *remote_contexts = new StartRemoteContexts (ref, registry, "filename.xml");
	m_registry_servant->set_callback (yans::make_callback (&StartRemoteContexts::registered, 
							       remote_contexts));

	remote_contexts->set_started_callback (yans::make_callback (&SimulationContextFactory::started_cb, this));

	while (!m_started) {
		if (m_orb->work_pending ()) {
			m_orb->perform_work ();
		}
	}
	delete remote_contexts;
	// we are done starting the remote contexts !
}
SimulationContext 
SimulationContextFactory::lookup (std::string name)
{
	for (ContextsI i = m_contexts.begin (); i != m_contexts.end (); i++) {
		if (i->first.compare (name) == 0) {
			return i->second;
		}
	}
	::Remote::ComputingContext_ptr remote = m_registry_servant->_this ()->lookup (name.c_str ());
	if (!CORBA::is_nil (remote)) {
		SimulationContext local = SimulationContext (new SimulationContextImpl (remote));
		m_contexts.push_back(std::make_pair (name, 
						     local));
		return local;
	}
	assert (false);
	return 0;
}

void
SimulationContextFactory::one_context_stopped (bool finished, uint64_t time)
{
	if (finished) {
		m_n_finished++;
	} else {
		if (time == m_next_us) {
			m_n_next_stopped++;
		} else {
			// unexpected stop.
			m_n_unexpected++;
		}
	}
	m_n_stopped++;
}


void 
SimulationContextFactory::run (void)
{
	::Remote::StoppedCallback_ptr stopped_cb = m_stopped_cb_servant->_this ();

	m_next_us = m_now_us;
	while (true) {
		if (!m_scheduler->is_empty ()) {
			yans::Scheduler::EventKey key = m_scheduler->peek_next_key ();
			m_next_us = key.m_time;
		}
		for (ContextsI i = m_contexts.begin (); i != m_contexts.end (); i++) {
			i->second->peek_remote ()->start (stopped_cb);
		}
		m_n_stopped = 0;
		m_n_finished = 0;
		m_n_unexpected = 0;
		m_n_next_stopped = 0;
		while (m_n_stopped != m_contexts.size ()) {
			if (m_orb->work_pending ()) {
				m_orb->perform_work ();
			}
		}
		if (m_n_unexpected > 0) {
			std::cerr << "unexpected remote stopped simulation context !" << std::endl;
			break;
		} else if (m_n_finished == m_contexts.size ()) {
			break;
		}
		// we know we are not empty here so we do not
		// need to check if the event is non-0
		if (!m_scheduler->is_empty ()) {
			yans::Event *ev = m_scheduler->peek_next ();
			yans::Scheduler::EventKey key = m_scheduler->peek_next_key ();
			m_scheduler->remove_next ();
			m_now_us = key.m_time;
			ev->invoke ();
			delete ev;
		}
	}
}

void 
SimulationContextFactory::insert_at_us (uint64_t us, Event *event)
{
	yans::Scheduler::EventKey key;
	key.m_time = us;
	key.m_uid = m_uid;
	m_scheduler->insert (yans::make_event (&yapns::Event::invoke, event), key);
	m_uid++;
}
uint64_t 
SimulationContextFactory::now_us (void)
{
	return m_now_us;
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
