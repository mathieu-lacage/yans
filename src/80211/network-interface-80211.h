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

#ifndef NETWORK_INTERFACE_80211_H
#define NETWORK_INTERFACE_80211_H

#include "network-interface.h"
#include "mac-address.h"
#include "ipv4-address.h"
#include <string>

namespace yans {

class Host;
class Packet;
class Ipv4;
class Channel80211;
class Phy80211;
class PropagationModel;
class MacStations;

class NetworkInterface80211 : public NetworkInterface {
public:
	virtual ~NetworkInterface80211 ();

	void connect_to (Channel80211 *channel);

	virtual void set_host (Host *host);
	virtual void set_mac_address (MacAddress self);
	virtual MacAddress get_mac_address (void);
	virtual std::string const *get_name (void);
	virtual uint16_t get_mtu (void);
	virtual void set_up   (void);
	virtual void set_down (void);
	virtual bool is_down (void);
	virtual void set_ipv4_handler (Ipv4 *ipv4);
	virtual void set_ipv4_address (Ipv4Address address);
	virtual void set_ipv4_mask    (Ipv4Mask mask);
	virtual Ipv4Address get_ipv4_address (void);
	virtual Ipv4Mask    get_ipv4_mask    (void);
	virtual Ipv4Address get_ipv4_broadcast (void);

	virtual void send (Packet *packet, Ipv4Address dest);

private:
	void rx_phy_ok (Packet *packet, double snr, uint8_t tx_mode);
	void rx_phy_error (Packet *packet);
	friend class NetworkInterface80211Factory;
	NetworkInterface80211 ();

	Host *m_host;
	Ipv4 *m_ipv4;

	PropagationModel *m_propagation;
	Phy80211 *m_phy;
	MacStations *m_stations;

	MacAddress m_bssid;
	MacAddress m_self;
	Ipv4Address m_ipv4_address;
	Ipv4Mask m_ipv4_mask;
	std::string *m_name;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_H */
