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
#ifndef MAC_LOW_80211_H
#define MAC_LOW_80211_H

#include "hdr-mac-80211.h"
#include "phy-80211.h"

class Packet;
class RngUniform;
class Backoff;
class Mac80211;
class MacQueue80211e;
class MacStation;
class DynamicMacHandler;
class MacCancelableEvent;
class StaticMacHandler;
class Phy80211;

class MacLow80211 {
public:
	MacLow80211 (Mac80211 *mac);
	~MacLow80211 ();

	void completeConstruction (Phy80211 *phy);

	void enqueue (Packet *packet);
	void receive (Packet *packet);

	Phy80211 *peekPhy80211 (void);

	double getEIFS (void);
	double getDIFS (void);

	int getSelf (void);

private:
	double now (void);
	MacStation *lookupStation (int address);

	int getACKSize (void) const;
	int getRTSSize (void) const;
	int getCTSSize (void) const;
	int getDestination (Packet *packet);
	int getSource (Packet *packet);
	int getTxMode (Packet *packet);
	enum mac_80211_packet_type getType (Packet *packet);

	int getDataHeaderSize (void);
	double getSIFS (void);
	double getSlotTime (void);
	int getCWmin (void);
	int getCWmax (void);
	
	Phy80211 *peekPhy (void);

	double getLastSNR (void);
	double getLastStartRx (void);
	void setSize (Packet *packet, int size);
	int getSize (Packet *packet);
	double getDuration (Packet *packet);
	void increaseSize (Packet *packet, int increment);
	void decreaseSize (Packet *packet, int decrement);
	double calculateTxDuration (int mode, int size);
	void setTxMode (Packet *packet, int mode);
	void setDestination (Packet *packet, int destination);	
	void setDuration (Packet *packet, double duration);
	void setSource (Packet *packet, int source);
	void setType (Packet *packet, enum mac_80211_packet_type type);
	void setRetry (Packet *packet);
	void initialize (Packet *packet);
	bool isRetry (Packet *packet);
	void setSequence (Packet *packet, int sequence);
	int getSequence (Packet *packet);
	Packet *getRTSPacket (void);
	Packet *getCTSPacket (void);
	Packet *getACKPacket (void);
	Packet *getRTSforPacket (Packet *data);
	void setExpectedCTSSource (int source);
	int getExpectedCTSSource (void);
	void setExpectedACKSource (int source);
	int getExpectedACKSource (void);
	double pickBackoffDelay (void);
	double pickBackoffDelayInCaseOfFailure (void);
	int calculateNewFailedCW (int CW);
	void resetCW (void);
	void updateFailedCW (void);
	void dropPacket (Packet *packet);
	void dropCurrentTxPacket (void);
	double max (double a, double b);

	int getMaxSSRC (void);
	int getMaxSLRC (void);
	int getRTSCTSThreshold (void);
	double getCTSTimeoutDuration (void);
	double getACKTimeoutDuration (void);
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
	MacQueue80211e *m_queue;
	DynamicMacHandler *m_ACKTimeoutBackoffHandler;
	DynamicMacHandler *m_CTSTimeoutBackoffHandler;
	StaticMacHandler *m_accessBackoffHandler;
	DynamicMacHandler *m_sendCTSHandler;
	DynamicMacHandler *m_sendACKHandler;
	DynamicMacHandler *m_sendDataHandler;
	Backoff *m_backoff;
	RngUniform *m_random;
};

#endif /* MAC_LOW_80211_H */
