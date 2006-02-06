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

#ifndef PHY_80211_H
#define PHY_80211_H

#include <list>
#include <vector>

#include "phy.h"

/* - an RX event means that a signal was sent to us but
 *   we have not synchronized on it because it is not 
 *   strong enough.
 *
 * - a SYNC event means that a signal was sent to us and
 *   that we have successfully synchronized on it. It does
 *   not mean that the packet has been successfully received
 *   without error though.
 *
 * - a TX event means that we are trying to send a packet
 *   to someone else.
 */

class Propagation;
class Antenna;


class EndRxHandler;
class TransmissionMode;
class PhyRxEvent;
class RngUniform;
class Phy80211EventList;


class Phy80211 : public Phy 
{
public:
	enum Phy80211State {
		SYNC,
		TX,
		IDLE,
		SLEEP
	};

	Phy80211 ();
	virtual ~Phy80211 ();

	/* override from Phy (mac/phy.h) */
	virtual void recv(Packet* p, Handler*);
	virtual void sendDown(Packet *p);
	virtual int sendUp(Packet *p);

	/* inherited from NsObject (common/object.h) */
	virtual int command(int argc, char const*const* argv);

	/* inherited from TclObject in tclcl (tclcl/tclcl.h) */
	virtual int delay_bind_dispatch (char const *varName, char const *localName, class TclObject *tracer);
	virtual void delay_bind_init_all ();

	void sleep (void);
	void wakeup (void);

	Phy80211EventList *peekEventList (void) const;

	enum Phy80211State getState (void);
	double getStateDuration (void);
	double getDelayUntilIdle (void);

	double getLastRxSNR (void);
	double getLastRxStartTime (void);

	double calculateTxDuration (Packet *packet);
	double calculateTxDuration (int payloadMode, int size);
private:
	friend class PhyRxEvent;
	class NIChange {
	public:
		NIChange (double time, double delta);
		double getTime (void);
		double getDelta (void);
		bool operator < (NIChange a) const;
	private:
		double m_time;
		double m_delta;
	};
	unsigned int m_standard;

	double       m_frequency;
	double       m_plcpPreambleDelay;
	uint32_t     m_plcpHeaderLength;

	double       m_systemLoss;
	double       m_rxThreshold;
	double       m_rxNoise;
	double       m_txPower; // XXX
	double       m_maxPacketDuration;

	void switchToTx (double txDuration);
	void switchToSyncFromIdle (void);
	void switchToSleep (void);
	void switchToIdleFromSleep (void);
	void switchToIdleFromSync (void);

	void startTx (Packet *p);
	void startRx (Packet *p);
	void endRx (PhyRxEvent *phyRxEvent, Packet *packet);
	void addTxRxMode (TransmissionMode *mode);
	Channel *peekChannel (void);
	MobileNode *peekMobileNode (void);
	double getPreambleDuration (void);
	double calculateHeaderDuration (int headerMode);
	double calculatePacketDuration (int headerMode, int payloadMode, int size);
	double calculateNoiseFloor (double signalSpread);
	double SNR (double signal, double noiseInterference, TransmissionMode *mode);
	double calculateCurrentNoiseInterference (void);
	double calculateNoiseInterference (double time);
	double calculatePower (Packet *p);
	double calculateNI (PhyRxEvent *phyRxEvent, vector <NIChange> *snir);
	double calculatePER (PhyRxEvent *packet, vector <NIChange> *snir);
	TransmissionMode *getMode (int mode);
	double calculateChunkSuccessRate (double snir, double delay, TransmissionMode *mode);
	void appendEvent (PhyRxEvent *event);

	int    getSize (Packet *packet);
	int    getPayloadMode (Packet *packet);
	int    getHeaderMode (Packet *packet);
	double dBmToW (double dBm);
	double dBToRatio (double dB);
	double getSystemLoss (void);
	double getLambda (void);
	double now (void);
	char const *stateToString (enum Phy80211State state);

	bool isDefined (char const *varName);
	bool isClassDefined (char const *className, char const *varName);
	void delay_bind_safe (char const *varName);

	
	Propagation *m_propagation;
	Antenna     *m_antenna;
	EndRxHandler *m_endRxHandler;
	RngUniform *m_random;
	Phy80211EventList *m_eventList;
	vector<class TransmissionMode *> m_modes;
	list<PhyRxEvent *> m_rxEventList;

	double m_rxStartSNR;
	double m_rxStartTime;
	bool m_sleeping;
	bool m_rxing;
	double m_endTx;
	double m_previousStateChangeTime;
};


#endif /* PHY_80211_H */
