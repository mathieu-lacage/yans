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

#ifndef PHY_80211_H
#define PHY_80211_H

#include <stdint.h>
#include <vector>

class FreeSpacePropagation;
class TransmissionMode;
class MacLow;
class NetInterface;
class Packet;

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



class Phy80211
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

	void setInterface (NetInterface *interface);
	
	void setMac (MacLow *low);

	void sendDown (Packet *p);
	void sendUp (Packet *p);

	void sleep (void);
	void wakeup (void);

	void registerListener (Phy80211Listener *listener);

	enum Phy80211State getState (void);
	double getStateDuration (void);
	double getDelayUntilIdle (void);

	double getLastRxSNR (void);

	double calculateTxDuration (Packet *packet);
	double calculateTxDuration (int payloadMode, int size);

	void configureStandardA (void);
	void setRxThreshold (double rxThreshold);
	void setRxNoise (double rxNoise);
	void setTxPower (double txPower);

	void setPropagationModel (FreeSpacePropagation *propagation);

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
	double calculateRxPower (Packet *p);
	double dBmToW (double dBm);
	double getRxThreshold (void);
	int    getPayloadMode (Packet *packet);
	double getMaxPacketDuration (void);
	double calculatePacketDuration (int headerMode, int payloadMode, int size);
	double getPreambleDuration (void);
	double calculateHeaderDuration (int headerMode);
	TransmissionMode *getMode (int mode);
	void setLastRxSNR (double snr);
	int selfAddress (void);
	double SNR (double signal, double noiseInterference, TransmissionMode *mode);
	void forwardUp (Packet *packet);
private:
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

	void addTxRxMode (TransmissionMode *mode);

	double calculateNoiseFloor (double signalSpread);
	int    getHeaderMode (Packet *packet);
	double dBToRatio (double dB);
	double getSystemLoss (void);
	double getLambda (void);
	double max (double a, double b);
	char const *stateToString (enum Phy80211State state);

	void notifyTxStart (double now, double duration);
	void notifySleep (double now);
	void notifyWakeup (double now);

	FreeSpacePropagation *m_propagation;
	typedef std::vector<Phy80211Listener *> Listeners;
	typedef std::vector<Phy80211Listener *>::iterator ListenersCI;
	typedef std::vector<class TransmissionMode *> Modes;
	typedef std::vector<class TransmissionMode *>::iterator ModesI;
	Listeners m_listeners;
	Modes m_modes;
	MacLow *m_mac;
	NetInterface *m_interface;

	double m_rxStartSNR;
	bool m_sleeping;
	bool m_rxing;
	double m_endTx;
	double m_endRx;
	double m_previousStateChangeTime;
};


#endif /* PHY_80211_H */
