/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "local-object-registry.h"

LocalObjectRegistry *
LocalObjectRegistry::instance (void)
{
	static LocalObjectRegistry * self = new LocalObjectRegistry ();
	return self;
}

PortableServer::POA_var
LocalObjectRegistry::get_root_poa (void)
{
  CORBA::Object_var poa_obj = m_orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
  return poa;
}


void 
LocalObjectRegistry::set_orb (CORBA::ORB_var orb)
{
	m_orb = orb;
}
void 
LocalObjectRegistry::set_registry (::Registry_var registry)
{
	m_registry = registry;
}
CORBA::ORB_var 
LocalObjectRegistry::get_orb (void)
{
	return m_orb;
}

void 
LocalObjectRegistry::record (uint64_t id, yans::StaticPosition *position)
{
	LocalObjectRegistry *self = instance ();
	assert (lookup (id) == 0);
	self->m_static_positions.push_back (std::make_pair (id, position));
}

yans::StaticPosition *
LocalObjectRegistry::lookup_static_position (uint64_t id)
{
	LocalObjectRegistry *self = instance ();
	for (StaticPositionsI i = self->m_static_positions.begin (); i != self->m_static_positions.end (); i++) {
		if (i->first == id) {
			return i->second;
			break;
		}
	}
	return 0;
}

