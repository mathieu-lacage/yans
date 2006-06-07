/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef LOCAL_OBJECT_REGISTRY_H
#define LOCAL_OBJECT_REGISTRY_H

#include "yans/static-position.h"
#include "context.h"
#include "registry.h"

class LocalObjectRegistry {
public:
	static LocalObjectRegistry *instance (void);

	void set_orb (CORBA::ORB_var orb);
	void set_registry (::Registry_var registry);
	CORBA::ORB_var get_orb (void);
	uint64_t get_next_id (void);

	void record (Remote::StaticPositionModel_ptr model, yans::StaticPosition *position);
	yans::StaticPosition *lookup (Remote::StaticPositionModel_ptr model);
private:
	typedef std::list<std::pair<Remote::StaticPositionModel_ptr, yans::StaticPosition *> > StaticPositions;
	typedef std::list<std::pair<Remote::StaticPositionModel_ptr, yans::StaticPosition *> >::iterator StaticPositionsI;

	StaticPositions m_static_positions;
	CORBA::ORB_var m_orb;
	::Registry_var m_registry;
};


#endif /* LOCAL_OBJECT_REGISTRY_H */
