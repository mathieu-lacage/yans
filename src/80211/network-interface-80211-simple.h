/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#ifndef NETWORK_INTERFACE_80211_SIMPLE_H
#define NETWORK_INTERFACE_80211_SIMPLE_H

#include "yans/mac-network-interface.h"
#include "yans/mac-address.h"
#include "yans/ui-traced-variable.tcc"
#include "yans/packet.h"
#include <string>

namespace yans {

class Channel80211;
class Phy80211;
class PropagationModel;
class MacStations;
class TraceContainer;
class MacSimple;

class NetworkInterface80211Simple : public MacNetworkInterface {
public:
	virtual ~NetworkInterface80211Simple ();

	void connect_to (Channel80211 *channel);
	void register_trace (TraceContainer *container);

private:
	virtual void notify_up (void);
	virtual void notify_down (void);
	virtual void real_send (Packet packet, MacAddress to);

	void forward_data_up (Packet packet);
	friend class NetworkInterface80211SimpleFactory;
	NetworkInterface80211Simple (MacAddress address);

	PropagationModel *m_propagation;
	Phy80211 *m_phy;
	MacStations *m_stations;
	MacSimple *m_mac;
	MacAddress m_bssid;
	UiTracedVariable<uint32_t> m_bytes_rx;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_SIMPLE_H */
