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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef QAP_SCHEDULER_H
#define QAP_SCHEDULER_H

#include <list>

#include "hdr-mac-80211.h"
#include "mac-handler.tcc"

class MacContainer;
class TSpec;
class Packet;
class MyBeaconMacLowTransmissionListener;
class MyCfPollMacLowTransmissionListener;
class Phy80211;
class MacParameters;
class MacLow;
class MacCancelableEvent;
class MacHighQap;
class MacDcfParameters;
class Dcf;

class Txop {
public:
	Txop (int destination, TSpec *tspec);

	int getDestination (void) const;
	TSpec *getTSpec (void) const;

	bool operator == (Txop const &o) const;
private:
	int m_destination;
	TSpec *m_tspec;
};


class QapScheduler 
{
public:
	/* must connect to the MacLow's Nav listener. */
	QapScheduler (MacContainer *container);

	/* invoked by MacHigh. 
	 * return true if request is accepted.
	 * return false otherwise.
	 */
	bool addTsRequest (int destination, TSpec *tspec);
	bool delTsRequest (int destination, TSpec *tspec);
	void gotQosNull (Packet *packet);

	void storeEdcaParametersInPacket (Packet *packet);
	Dcf *createDcf (enum ac_e ac);

private:
	friend class MyBeaconMacLowTransmissionListener;
	friend class MyCfPollMacLowTransmissionListener;

	void gotCfPollAck (void);
	void missedCfPollAck (void);
	void capStartTimer (MacCancelableEvent *event);
	void beaconTxNextData (void);
	void beaconTimer (MacCancelableEvent *event);
	void txopStartTimer (MacCancelableEvent *event);


	void startCap (void);
	void finishCap (void);
	void startCurrentTxop (void);
	void nextTxop (void);

	void sendCfPollTo (int destination, uint8_t tsid, double duration);
	double getMaxTxopDuration (void);
	double duration (int size, int mode);
	double calculateMediumTime (TSpec const *tspec);
	double calculateTxopDuration (double serviceInterval, TSpec const *tspec);
	double getCurrentServiceInterval (void);
	void setCurrentServiceInterval (double serviceInterval);
	double getMaximumServiceInterval (TSpec *tspec);
	double calculateTotalCapTime (double serviceInterval);
	double getCurrentTotalCapTime (void);


	Phy80211 *phy (void);
	MacParameters *parameters (void);
	int getPacketSize (enum mac_80211_packet_type type);
	double now (void);
	MacLow *low (void);
	Packet *getPacketFor (int to);

	MacContainer *m_container;
	MacHighQap *m_high;
	MyBeaconMacLowTransmissionListener *m_beaconTxListener;
	MyCfPollMacLowTransmissionListener *m_cfPollTxListener;
	DynamicHandler<QapScheduler> *m_capStart;
	DynamicHandler<QapScheduler> *m_beacon;
	DynamicHandler<QapScheduler> *m_txopStart;

	std::list<Txop> m_admitted;
	std::list<Txop>::const_iterator m_txopIterator;
	double m_txopEndTime;
	double m_capEndTime;
	double m_capStartTime;
	double m_nextBeaconStartTime;
	double m_currentServiceInterval;
	uint32_t m_sequence;
	MacDcfParameters *m_dcfParameters[4];
};

#endif /* QAP_SCHEDULER_H */
