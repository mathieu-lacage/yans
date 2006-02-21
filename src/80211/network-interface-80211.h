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

namespace yans {

class MacHigh;
class MacLow;
class MacParameters;
class MacTxMiddle;
class MacRxMiddle;
class MacStations;
class Phy80211;
class Packet;

class NetworkInterface80211 : public NetworkInterface {
public:
	virtual ~NetworkInterface80211 ();

	MacAddress get_bssid (void);	

	MacParameters *get_parameters (void);
	MacTxMiddle *get_tx_middle (void);
	MacRxMiddle *get_rx_middle (void);
	MacHigh *get_high (void);
	MacLow *get_low (void);
	MacStations *get_stations (void);
	Phy80211 *get_phy (void);
	


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

	friend class NetInterfaceConstructor80211;

	NetInterface80211 ();

#if 0
	BroadcastChannel *m_channel;
	LLArp *m_ll;
	MacHigh *m_high;
	MacLow *m_low;
	MacTxMiddle *m_txMiddle;
	MacRxMiddle *m_rxMiddle;
	MacParameters *m_parameters;
	MacStations *m_stations;
	Phy80211 *m_phy;
#endif
	Host *m_host;
	MacAddress m_bssid;
	MacAddress m_self;
};

class Ssid {
public:
	Ssid (char const[14] ssid);
private:
	char const m_ssid[14];
};


class NetworkInterface80211Factory {
public:
	NetworkInterface80211Factory ();
	~NetInterface80211Factory ();

	void set_ssid (Ssid const ssid);

	void set_qap (void);
	void set_qsta (void);
	void set_nqap (void);
	void set_nqsta (void);
	void set_adhoc (void);

	void set_arf (void);
	void set_aarf (void);
	void set_cr (int data_mode, int ctl_mode);

	void set_80211a (void);
	/*void set80211b (void);*/

	/* absolute reception threshold. dBm. */
	void setPhyRxTreshold (double rxThreshold);
	/* Ratio of energy lost by receiver. dB. */
	void setPhyRxNoise (double rxNoise);
	/* absolute transmission energy. dBm. */
	void setPhyTxPower (double txPower);
	void setPhyModelBer (void);
	void setPhyModelSnrt (void);
	void setPhyModelEt (void);

	/* absolute system loss. W. */
	void setPropSystemLoss (double systemLoss);
	void setPropTxGain (double txGain);
	void setPropRxGain (double rxGain);
	void setPropFrequency (double frequency);
	void setFreeSpacePropagationModel (void);


	NetworkInterface80211 *create (void);

private:
	enum {
		MAC_80211_MODE_QAP,
		MAC_80211_MODE_NQAP,
		MAC_80211_MODE_QSTA,
		MAC_80211_MODE_NQSTA,
		MAC_80211_MODE_ADHOC
	} m_mac_mode;

	enum {
		STANDARD_80211_A
		/* This is not really implemented but it should be
		   trivial to do.
		   STANDARD_80211_MODE_B 
		*/
	} m_standard;

	enum {
		MAC_80211_RATE_ARF,
		MAC_80211_RATE_AARF,
		MAC_80211_RATE_CR
	} m_rate_control_mode;

	enum {
		PHY_80211_MODEL_BER,
		PHY_80211_MODEL_SNRT,
		PHY_80211_MODEL_ET
	} m_phy_model;

	int m_ap_mac_address;
	
	double m_prop_system_loss;
	double m_prop_tx_gain;
	double m_propRxGain;
	double m_propFrequency;

	double m_phyRxThreshold;
	double m_phyRxNoise;
	double m_phyTxPower;

	int m_cr_data_mode;
	int m_cr_ctl_mode;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_H */
