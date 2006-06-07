/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "local-object-registry.h"

LocalObjectRegistry *
LocalObjectRegistry::instance (void)
{
	static LocalObjectRegistry * self = new LocalObjectRegistry ();
	return self;
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
uint64_t 
LocalObjectRegistry::get_next_id (void)
{
	return m_registry->get_id ();
}


void 
LocalObjectRegistry::record (Remote::StaticPositionModel_ptr model, yans::StaticPosition *position)
{
	assert (lookup (model) == 0);
	m_static_positions.push_back (std::make_pair (model, position));
}

yans::StaticPosition *
LocalObjectRegistry::lookup (Remote::StaticPositionModel_ptr model)
{
	for (StaticPositionsI i = m_static_positions.begin (); i != m_static_positions.end (); i++) {
		if (i->first->get_id () == model->get_id ()) {
			return i->second;
			break;
		}
	}
	return 0;
}

