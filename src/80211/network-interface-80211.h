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

#include "mac-network-interface.h"
#include "mac-address.h"
#include "ipv4-address.h"
#include "ssid.h"
#include "packet.h"
#include <string>

namespace yans {

class BaseChannel80211;
class Phy80211;
class PropagationModel;
class MacStations;
class TraceContainer;
class MacLow;
class MacRxMiddle;
class MacTxMiddle;
class MacHighAdhoc;
class MacParameters;
class Dcf;
class DcaTxop;
class MacQueue80211e;
class MacHighAdhoc;
class MacHighNqsta;
class MacHighNqap;
class PacketLogger;

class NetworkInterface80211 : public MacNetworkInterface {
public:
	virtual ~NetworkInterface80211 ();

	void connect_to (BaseChannel80211 *channel);
	void register_traces (TraceContainer *container);

	virtual MacAddress get_bssid (void) const = 0;
	virtual Ssid get_ssid (void) const = 0;


protected:
	NetworkInterface80211 (MacAddress address);
	void forward_up_data (PacketPtr packet);
private:
	virtual void notify_up (void);
	virtual void notify_down (void);
	virtual void real_send (PacketPtr packet, MacAddress to) = 0;
	void associated (void);

	friend class NetworkInterface80211Factory;

	PropagationModel *m_propagation;
	Phy80211 *m_phy;
	MacStations *m_stations;
	MacLow *m_low;
	MacRxMiddle *m_rx_middle;
	MacTxMiddle *m_tx_middle;
	MacParameters *m_parameters;
	PacketLogger *m_rx_logger;
};

class NetworkInterface80211Adhoc : public NetworkInterface80211 {
public:
	NetworkInterface80211Adhoc (MacAddress address);
	virtual ~NetworkInterface80211Adhoc ();

	virtual MacAddress get_bssid (void) const;
	virtual Ssid get_ssid (void) const;
	void set_ssid (Ssid ssid);
private:
	virtual void real_send (PacketPtr packet, MacAddress to);
	friend class NetworkInterface80211Factory;
	Ssid m_ssid;
	DcaTxop *m_dca;
	MacHighAdhoc *m_high;
};

class NetworkInterface80211Nqsta : public NetworkInterface80211 {
public:
	NetworkInterface80211Nqsta (MacAddress address);
	virtual ~NetworkInterface80211Nqsta ();

	virtual MacAddress get_bssid (void) const;
	virtual Ssid get_ssid (void) const;
	void start_active_association (Ssid ssid);
private:
	void associated (void);
	virtual void real_send (PacketPtr packet, MacAddress to);
	friend class NetworkInterface80211Factory;
	Ssid m_ssid;
	DcaTxop *m_dca;
	MacHighNqsta *m_high;
};

class NetworkInterface80211Nqap : public NetworkInterface80211 {
public:
	NetworkInterface80211Nqap (MacAddress address);
	virtual ~NetworkInterface80211Nqap ();

	virtual MacAddress get_bssid (void) const;
	virtual Ssid get_ssid (void) const;
	void set_ssid (Ssid ssid);
	
private:
	virtual void real_send (PacketPtr packet, MacAddress to);
	friend class NetworkInterface80211Factory;
	Ssid m_ssid;
	DcaTxop *m_dca;
	MacHighNqap *m_high;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_H */
