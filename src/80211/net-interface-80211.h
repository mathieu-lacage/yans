/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef NET_INTERFACE_80211_H
#define NET_INTERFACE_80211_H

#include "net-interface.h"
#include "net-node.h"

class LLArp;
class MacHigh;
class MacLow;
class MacParameters;
class MacTxMiddle;
class MacRxMiddle;
class MacStations;
class Phy80211;
class Packet;

class NetInterface80211 : public NetInterface {
public:
	virtual ~NetInterface80211 ();

	int getBSSID (void);

	

	MacParameters *parameters (void);
	MacTxMiddle *txMiddle (void);
	MacRxMiddle *rxMiddle (void);
	MacHigh *high (void);
	MacLow *low (void);
	MacStations *stations (void);
	Phy80211 *phy (void);
	LLArp *ll (void);
	
	virtual void connectTo (BroadcastChannel *channel, NetNode *node);
	virtual void sendUp (Packet *packet);
	virtual void sendDown (Packet *packet);
	virtual void sendUpToNode (Packet *packet);
	virtual void sendDownToChannel (Packet *packet);
	virtual int32_t getIpAddress (void);
	virtual int getMacAddress (void);
	virtual void peekPosition (NodePosition *position);

private:

	friend class NetInterfaceConstructor80211;

	NetInterface80211 ();

	NetNode *m_node;
	BroadcastChannel *m_channel;

	LLArp *m_ll;
	MacHigh *m_high;
	MacLow *m_low;
	MacTxMiddle *m_txMiddle;
	MacRxMiddle *m_rxMiddle;
	MacParameters *m_parameters;
	MacStations *m_stations;
	Phy80211 *m_phy;
	int m_bssid;
	NodePosition m_position;
};


class NetInterfaceConstructor80211 : public NetInterfaceConstructor {
public:
	NetInterfaceConstructor80211 ();
	virtual ~NetInterfaceConstructor80211 ();


	void setQap (void);
	void setQsta (int ap);
	void setNqap (void);
	void setNqsta (int ap);
	void setAdhoc (void);

	void setArf (void);
	void setAarf (void);
	void setCr (int dataMode, int ctlMode);

	void set80211a (void);
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

	void setArpAliveTimeout (double aliveTimeout);
	void setArpDeadTimeout (double deadTimeout);
	void setArpWaitReplyTimeout (double waitReplyTimeout);

	NetInterface80211 *createInterface (void);

private:
	enum {
		MAC_80211_MODE_QAP,
		MAC_80211_MODE_NQAP,
		MAC_80211_MODE_QSTA,
		MAC_80211_MODE_NQSTA,
		MAC_80211_MODE_ADHOC
	} m_macMode;

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
	} m_rateControlMode;

	enum {
		PHY_80211_MODEL_BER,
		PHY_80211_MODEL_SNRT,
		PHY_80211_MODEL_ET
	} m_phyModel;

	int m_apMacAddress;
	
	double m_propSystemLoss;
	double m_propTxGain;
	double m_propRxGain;
	double m_propFrequency;

	double m_phyRxThreshold;
	double m_phyRxNoise;
	double m_phyTxPower;

	int m_crDataMode;
	int m_crCtlMode;

	double m_arpAliveTimeout;
	double m_arpDeadTimeout;
	double m_arpWaitReplyTimeout;
};

#endif /* NET_INTERFACE_80211_H */
