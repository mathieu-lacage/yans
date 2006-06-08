/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef LOCAL_OBJECT_REGISTRY_H
#define LOCAL_OBJECT_REGISTRY_H

#include "yans/static-position.h"
#include "context.h"
#include "registry.h"

class LocalObjectRegistry {
public:
	static LocalObjectRegistry *instance (void);

	static void record_static_position (uint64_t id, yans::StaticPosition *position);
	static void record_callback_void_packet (uint64_t id, yans::Callback<void,yans::Packet *> cb);
	static void record_channel_80211 (uint64_t id, yans::BaseChannel80211 *real_channel);
	static void record_mac_interface (uint64_t id, yans::MacNetworkInterface *real_interface);
	static void record_ipv4_interface (uint64_t id, yans::Ipv4NetworkInterface *real_interface);
	static yans::StaticPosition *lookup_static_position (uint64_t id);
	static yans::Callback<void, Packet *>lookup_callback_void_packet (uint64_t id);
	static yans::BaseChannel80211 *lookup_channel_80211 (uint64_t id);
	static yans::MacNetworkInterface *lookup_mac_interface (uint64_t id);
	static yans::Ipv4NetworkInterface *lookup_ipv4_interface (uint64_t id);
private:
	typedef std::list<std::pair<uint64_t, yans::StaticPosition *> > StaticPositions;
	typedef std::list<std::pair<uint64_t, yans::StaticPosition *> >::iterator StaticPositionsI;

	StaticPositions m_static_positions;
	CORBA::ORB_var m_orb;
	::Registry_var m_registry;
};


#endif /* LOCAL_OBJECT_REGISTRY_H */
