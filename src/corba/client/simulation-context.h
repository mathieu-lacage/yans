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
#include <vector>
#include "yans/reference-list.h"
#include "remote-context.h"
#include "event.h"

namespace yans {
class SchedulerList;
};

class StoppedCallback_impl;
class Registry_impl;

namespace yapns {

class SimulationContextImpl;

typedef yans::ReferenceList<SimulationContextImpl *> SimulationContext;

class SimulationContextFactory {
public:
	SimulationContextFactory ();
	~SimulationContextFactory ();
	void initialize (int argc, char *argv[]);
	void read_configuration (char const *filename);
	SimulationContext lookup (std::string name);
	void run (void);
	void insert_at_us (uint64_t us, Event *event);
	uint64_t now_us (void);
private:
	void activate_servant (PortableServer::StaticImplementation *);
	void one_context_stopped (bool finished, uint64_t time);
	void started_cb (void);
	std::string lookup_remote_name (std::string const name);
	typedef std::list<std::pair<std::string, SimulationContext> > Contexts;
	typedef std::list<std::pair<std::string, SimulationContext> >::iterator ContextsI;


	CORBA::ORB_var m_orb;
	bool m_started;
	bool m_finished;
	uint32_t m_n_finished;
	uint32_t m_n_next_stopped;
	uint32_t m_n_unexpected;
	uint32_t m_n_stopped;
	uint64_t m_now_us;
	uint64_t m_next_us;
	uint32_t m_uid;
	yans::SchedulerList *m_scheduler;
	StoppedCallback_impl *m_stopped_cb_servant;
	Registry_impl *m_registry_servant;
	Contexts m_contexts;
};

class SimulationContextImpl {
public:
	SimulationContextImpl (::Remote::ComputingContext_ptr);
	~SimulationContextImpl ();
	bool is_equal (SimulationContext o);
	::Remote::ComputingContext_ptr peek_remote (void) const;
	::Remote::NetworkInterface80211Factory_ptr peek_80211_factory (void);
private:
	::Remote::ComputingContext_ptr m_context;
	::Remote::NetworkInterface80211Factory_ptr m_80211_factory;
};

}; // namespace yapns

#endif /* YAPNS_SIMULATION_CONTEXT_H */
