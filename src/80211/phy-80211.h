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

#include <vector>

#include "phy.h"


class Propagation;
class Antenna;
class TransmissionMode;

class Phy80211Listener {
public:
	virtual ~Phy80211Listener ();

	/* we have received the first bit of a packet. We decided
	 * that we could synchronize on this packet. It does not mean
	 * we will be able to successfully receive completely the
	 * whole packet. It means we will report a BUSY status.
	 * rxEnd will be invoked later to report whether or not
	 * the packet was successfully received.
	 */
	virtual void notifyRxStart (double now, double duration) = 0;
	/* we have received the last bit of a packet for which
	 * rxStart was invoked first. 
	 */
	virtual void notifyRxEnd (double now, bool receivedOk) = 0;
	/* we start the transmission of a packet.
	 */
	virtual void notifyTxStart (double now, double duration) = 0;
	virtual void notifySleep (double now) = 0;
	virtual void notifyWakeup (double now) = 0;
};



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

	void sleep (void);
	void wakeup (void);

	void registerListener (Phy80211Listener *listener);

	enum Phy80211State getState (void);
	double getStateDuration (void);
	double getDelayUntilIdle (void);

	double getLastRxSNR (void);

	double calculateTxDuration (Packet *packet);
	double calculateTxDuration (int payloadMode, int size);

	int getNModes (void);
private:
	virtual void startRx (Packet *packet) = 0;
	virtual void cancelRx (void) = 0;
protected:
	void notifyRxStart (double now, double duration);
	void notifyRxEnd (double now, bool receivedOk);
	void switchToSyncFromIdle (double rxDuration);
	void switchToIdleFromSync (void);
	double now (void);
	double calculatePower (Packet *p);
	double dBmToW (double dBm);
	double getRxThreshold (void);
	int    getPayloadMode (Packet *packet);
	double getMaxPacketDuration (void);
	double calculatePacketDuration (int headerMode, int payloadMode, int size);
	double getPreambleDuration (void);
	double calculateHeaderDuration (int headerMode);
	TransmissionMode *getMode (int mode);
	double calculateNoiseFloor (double signalSpread);
	void setLastRxSNR (double snr);
	int selfAddress (void);
private:
	enum {
		standard_80211_unknown = 0,
		standard_80211_b = 1,
		standard_80211_a = 2,
	};
	uint32_t     m_standard;

	double       m_frequency;
	double       m_plcpPreambleDelay;
	uint32_t     m_plcpHeaderLength;

	double       m_systemLoss;
	double       m_rxThreshold;
	double       m_rxNoise;
	double       m_txPower; // XXX
	double       m_maxPacketDuration;

	void switchToTx (double txDuration);
	void switchToSleep (void);
	void switchToIdleFromSleep (void);

	double getEndOfTx (void);
	double getEndOfRx (void);

	void startTx (Packet *p);
	void addTxRxMode (TransmissionMode *mode);
	Channel *peekChannel (void);
	MobileNode *peekMobileNode (void);

	int    getHeaderMode (Packet *packet);
	double dBToRatio (double dB);
	double getSystemLoss (void);
	double getLambda (void);
	double max (double a, double b);
	char const *stateToString (enum Phy80211State state);

	bool isDefined (char const *varName);
	void define (char const *varName, uint32_t defaultValue);
	void initialize (char const *varName, uint32_t *variable, uint32_t defaultValue);


	void notifyTxStart (double now, double duration);
	void notifySleep (double now);
	void notifyWakeup (double now);

	
	Propagation *m_propagation;
	Antenna     *m_antenna;
	vector<Phy80211Listener *> m_listeners;
	vector<class TransmissionMode *> m_modes;

	double m_rxStartSNR;
	bool m_sleeping;
	bool m_rxing;
	double m_endTx;
	double m_endRx;
	double m_previousStateChangeTime;
};


#endif /* PHY_80211_H */
