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

#include "network-interface.h"
#include "mac-address.h"
#include "ipv4-address.h"
#include "ui-traced-variable.tcc"
#include <string>

namespace yans {

class Host;
class Packet;
class Ipv4;
class Channel80211;
class Phy80211;
class PropagationModel;
class MacStations;
class TraceContainer;
class MacSimple;
class Arp;

class NetworkInterface80211Simple : public NetworkInterface {
public:
	virtual ~NetworkInterface80211Simple ();

	void connect_to (Channel80211 *channel);
	void register_trace (TraceContainer *container);

	virtual void set_host (Host *host);
	virtual void set_mac_address (MacAddress self);
	virtual MacAddress get_mac_address (void) const;
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
	void forward_data_up (Packet *packet);
	void send_arp (Packet *packet, MacAddress to);
	void send_data (Packet *packet, MacAddress to);
	friend class NetworkInterface80211SimpleFactory;
	NetworkInterface80211Simple ();

	enum {
		ETHER_TYPE_IPV4 = 0x0800,
		ETHER_TYPE_ARP  = 0x0806
	};


	Host *m_host;
	Ipv4 *m_ipv4;

	PropagationModel *m_propagation;
	Phy80211 *m_phy;
	MacStations *m_stations;
	MacSimple *m_mac;
	Arp *m_arp;

	MacAddress m_bssid;
	MacAddress m_self;
	Ipv4Address m_ipv4_address;
	Ipv4Mask m_ipv4_mask;
	std::string *m_name;

	UiTracedVariable<uint32_t> m_bytes_rx;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_SIMPLE_H */
