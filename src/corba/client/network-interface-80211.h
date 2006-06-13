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
#ifndef YAPNS_NETWORK_INTERFACE_80211_H
#define YAPNS_NETWORK_INTERFACE_80211_H

#include "mac-network-interface.h"
#include "simulation-context.h"

namespace yapns {

class Channel80211;

class NetworkInterface80211 : public MacNetworkInterface {
public:
	NetworkInterface80211 (SimulationContext ctx);
	virtual ~NetworkInterface80211 ();

	SimulationContext get_context (void);
	::Remote::NetworkInterface80211_ptr get_remote_80211 (void);

	void connect_to (Channel80211 *channel);
private:
	virtual ::Remote::NetworkInterface80211_ptr real_get_remote_80211 (void) = 0;
	SimulationContext m_context;
};


class NetworkInterface80211Adhoc : public NetworkInterface80211 {
public:
	NetworkInterface80211Adhoc (SimulationContext ctx, ::Remote::NetworkInterface80211Adhoc_ptr remote);
	virtual ~NetworkInterface80211Adhoc ();

private:
	virtual ::Remote::MacNetworkInterface_ptr real_get_remote (void);
	virtual ::Remote::NetworkInterface80211_ptr real_get_remote_80211 (void);
	::Remote::NetworkInterface80211Adhoc_ptr m_remote;
};

}; // namespace yapns


#endif /* YAPNS_NETWORK_INTERFACE_80211_H */
