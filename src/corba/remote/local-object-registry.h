/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef LOCAL_OBJECT_REGISTRY_H
#define LOCAL_OBJECT_REGISTRY_H

#include "yans/callback.h"
#include "yans/stdint.h"
#include <list>

namespace yans {
class Packet;
class StaticPosition;
class BaseChannel80211;
class MacNetworkInterface;
class Ipv4NetworkInterface;
class Host;
};

class LocalObjectRegistry {
public:
	static LocalObjectRegistry *instance (void);

	static void record_static_position (uint64_t id, yans::StaticPosition *position);
	static void record_callback_void_packet (uint64_t id, yans::Callback<void,yans::Packet *> cb);
	static void record_channel_80211 (uint64_t id, yans::BaseChannel80211 *real_channel);
	static void record_mac_interface (uint64_t id, yans::MacNetworkInterface *real_interface);
	static void record_ipv4_interface (uint64_t id, yans::Ipv4NetworkInterface *real_interface);
	static void record_node (uint64_t id, yans::Host *real_node);
	static yans::StaticPosition *lookup_static_position (uint64_t id);
	static yans::Callback<void, yans::Packet *>lookup_callback_void_packet (uint64_t id);
	static yans::BaseChannel80211 *lookup_channel_80211 (uint64_t id);
	static yans::MacNetworkInterface *lookup_mac_interface (uint64_t id);
	static yans::Ipv4NetworkInterface *lookup_ipv4_interface (uint64_t id);
	static yans::Host *lookup_node (uint64_t id);
private:
	typedef std::list<std::pair<uint64_t, yans::StaticPosition *> > StaticPositions;
	typedef std::list<std::pair<uint64_t, yans::StaticPosition *> >::iterator StaticPositionsI;
	typedef std::list<std::pair<uint64_t, yans::Callback<void,yans::Packet *> > > CallbackVoidPacketList;
	typedef std::list<std::pair<uint64_t, yans::Callback<void,yans::Packet *> > >::iterator CallbackVoidPacketListI;
	typedef std::list<std::pair<uint64_t, yans::BaseChannel80211 *> > Channel80211s;
	typedef std::list<std::pair<uint64_t, yans::BaseChannel80211 *> >::iterator Channel80211sI;
	typedef std::list<std::pair<uint64_t, yans::MacNetworkInterface *> > MacNetworkInterfaces;
	typedef std::list<std::pair<uint64_t, yans::MacNetworkInterface *> >::iterator MacNetworkInterfacesI;
	typedef std::list<std::pair<uint64_t, yans::Ipv4NetworkInterface *> > Ipv4NetworkInterfaces;
	typedef std::list<std::pair<uint64_t, yans::Ipv4NetworkInterface *> >::iterator Ipv4NetworkInterfacesI;
	typedef std::list<std::pair<uint64_t, yans::Host *> > Nodes;
	typedef std::list<std::pair<uint64_t, yans::Host *> >::iterator NodesI;

	

	LocalObjectRegistry ();

	StaticPositions m_static_positions;
	CallbackVoidPacketList m_callbacks;
	Channel80211s m_channels;
	MacNetworkInterfaces m_mac_interfaces;
	Ipv4NetworkInterfaces m_ipv4_interfaces;
	Nodes m_nodes;
};


#endif /* LOCAL_OBJECT_REGISTRY_H */
