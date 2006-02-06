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
#ifndef MAC_LOW_H
#define MAC_LOW_H

#include "hdr-mac-80211.h"
#include "phy-80211.h"
#include "static-handler.tcc"

class Packet;
class RngUniform;
class Backoff;
class Mac80211;
class MacQueue80211e;
class MacStation;
class MacHigh;
class DynamicMacHandler;
class MacCancelableEvent;
class MacHigh;
class Phy80211;
class MacLowParameters;

class MacLow {
public:
	MacLow (Mac80211 *mac, MacHigh *high, Phy80211 *phy, 
		MacLowParameters *parameters);
	~MacLow ();

	void enqueue (Packet *packet);
	void receive (Packet *packet);
	void flush (void);

	Phy80211 *peekPhy (void);

	double getEIFS (void);
	double getDIFS (void);

	int getSelf (void);

private:
	MacLowParameters *parameters (void);
	void forwardDown (Packet *packet);
	double now (void);
	MacStation *lookupStation (int address);

	bool isData (Packet *packet);
	bool isManagement (Packet *packet);
	
	double getLastSNR (void);
	double getLastStartRx (void);
	double calculateTxDuration (int mode, int size);
	Packet *getRTSPacket (void);
	Packet *getCTSPacket (void);
	Packet *getACKPacket (void);
	Packet *getRTSforPacket (Packet *data);
	double pickBackoffDelay (void);
	double pickBackoffDelayInCaseOfFailure (void);
	int calculateNewFailedCW (int CW);
	void resetCW (void);
	void updateFailedCW (void);
	void dropPacket (Packet *packet);
	void dropCurrentTxPacket (void);
	double max (double a, double b);
	double getXIFSLeft (void);
	void increaseSequence (void);


	void sendRTSForPacket (Packet *packet);
	void sendDataPacket (Packet *packet);
	void sendCurrentTxPacket (void);
	void startTransmission (void);
	void dealWithInputQueue (void);

	/* the event handlers. */
	void ACKTimeout (MacCancelableEvent *event);
	void CTSTimeout (MacCancelableEvent *event);
	void sendCTS_AfterRTS (MacCancelableEvent *macEvent);
	void sendACK_AfterData (MacCancelableEvent *macEvent);
	void sendDataAfterCTS (MacCancelableEvent *macEvent);
	void initialBackoffTimeout (void);

	


	Packet *m_currentTxPacket;
	/* XXX init all */
	int m_sequence;
	int m_expectedCTSSource;
	int m_expectedACKSource;
	int m_CW;
	int m_SSRC;
	int m_SLRC;
	Mac80211 *m_mac;
	MacHigh *m_high;
	Phy80211 *m_phy;
	MacLowParameters *m_parameters;
	MacQueue80211e *m_queue;
	DynamicMacHandler *m_ACKTimeoutBackoffHandler;
	DynamicMacHandler *m_CTSTimeoutBackoffHandler;
	StaticHandler<MacLow> *m_accessBackoffHandler;
	DynamicMacHandler *m_sendCTSHandler;
	DynamicMacHandler *m_sendACKHandler;
	DynamicMacHandler *m_sendDataHandler;
	Backoff *m_backoff;
	RngUniform *m_random;
};

#endif /* MAC_LOW_H */
