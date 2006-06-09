/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "local-object-registry.h"
#include <cassert>


LocalObjectRegistry *
LocalObjectRegistry::instance (void)
{
	static LocalObjectRegistry * self = new LocalObjectRegistry ();
	return self;
}

LocalObjectRegistry::LocalObjectRegistry ()
{}


void 
LocalObjectRegistry::record_static_position (uint64_t id, yans::StaticPosition *position)
{
	LocalObjectRegistry *self = instance ();
	assert (lookup_static_position (id) == 0);
	self->m_static_positions.push_back (std::make_pair (id, position));
}

void 
LocalObjectRegistry::record_callback_void_packet (uint64_t id, yans::Callback<void,yans::Packet *> cb)
{
	LocalObjectRegistry *self = instance ();
	assert (lookup_callback_void_packet (id).is_null ());
	self->m_callbacks.push_back (std::make_pair (id, cb));
}
void
LocalObjectRegistry::record_channel_80211 (uint64_t id, yans::BaseChannel80211 *real_channel)
{
	LocalObjectRegistry *self = instance ();
	assert (lookup_channel_80211 (id) == 0);
	self->m_channels.push_back (std::make_pair (id, real_channel));
}
void 
LocalObjectRegistry::record_mac_interface (uint64_t id, yans::MacNetworkInterface *real_interface)
{
	LocalObjectRegistry *self = instance ();
	assert (lookup_mac_interface (id) == 0);
	self->m_mac_interfaces.push_back (std::make_pair (id, real_interface));
}
void 
LocalObjectRegistry::record_ipv4_interface (uint64_t id, yans::Ipv4NetworkInterface *real_interface)
{
	LocalObjectRegistry *self = instance ();
	assert (lookup_ipv4_interface (id) == 0);
	self->m_ipv4_interfaces.push_back (std::make_pair (id, real_interface));
}
void 
LocalObjectRegistry::record_node (uint64_t id, yans::Host *real_node)
{
	LocalObjectRegistry *self = instance ();
	assert (lookup_node (id) == 0);
	self->m_nodes.push_back (std::make_pair (id, real_node));
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

yans::Callback<void, yans::Packet *>
LocalObjectRegistry::lookup_callback_void_packet (uint64_t id)
{
	LocalObjectRegistry *self = instance ();
	for (CallbackVoidPacketListI i = self->m_callbacks.begin (); i != self->m_callbacks.end (); i++) {
		if (i->first == id) {
			return i->second;
			break;
		}
	}
	return yans::make_null_callback<void,yans::Packet *> ();
}
yans::BaseChannel80211 *
LocalObjectRegistry::lookup_channel_80211 (uint64_t id)
{
	LocalObjectRegistry *self = instance ();
	for (Channel80211sI i = self->m_channels.begin (); i != self->m_channels.end (); i++) {
		if (i->first == id) {
			return i->second;
			break;
		}
	}
	return 0;
}
yans::MacNetworkInterface *
LocalObjectRegistry::lookup_mac_interface (uint64_t id)
{
	LocalObjectRegistry *self = instance ();
	for (MacNetworkInterfacesI i = self->m_mac_interfaces.begin (); i != self->m_mac_interfaces.end (); i++) {
		if (i->first == id) {
			return i->second;
			break;
		}
	}
	return 0;
}
yans::Ipv4NetworkInterface *
LocalObjectRegistry::lookup_ipv4_interface (uint64_t id)
{
	LocalObjectRegistry *self = instance ();
	for (Ipv4NetworkInterfacesI i = self->m_ipv4_interfaces.begin (); i != self->m_ipv4_interfaces.end (); i++) {
		if (i->first == id) {
			return i->second;
			break;
		}
	}
	return 0;
}
yans::Host *
LocalObjectRegistry::lookup_node (uint64_t id)
{
	LocalObjectRegistry *self = instance ();
	for (NodesI i = self->m_nodes.begin (); i != self->m_nodes.end (); i++) {
		if (i->first == id) {
			return i->second;
			break;
		}
	}
	return 0;
}

