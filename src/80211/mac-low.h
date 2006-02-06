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
#include "mac-handler.tcc"

class Packet;
class Mac80211;
class MacStation;
class MacHigh;
class MacHigh;
class Phy80211;
class MacLowParameters;

class EventListener {
public:
	EventListener ();
	virtual ~EventListener ();
	void gotCTS (double snr);
	void missedCTS (void);
	void gotACK (double snr);
	void missedACK (void);
	void txCompletedAndSIFS (void);
	void gotBlockAckStart (double snr);
	void gotBlockAck (int sequence);
	void gotBlockAckEnd (void);
	void missedBlockAck (void);
};

class NavListener {
public:
	NavListener ();
	virtual ~NavListener ();
	void gotNav (double now, double nav);
};

class MacLow {
public:
	MacLow (Mac80211 *mac, MacHigh *high, Phy80211 *phy,
		MacLowParameters *parameters);
	~MacLow ();

	/* notify the EventListener after end-of-handshake+SIFS. 
	 * If ACK is enabled, end-of-handshake is defined to be 
	 * the end of reception of the ACK. Otherwise,
	 * it is the end of transmission of the main packet.
	 */
	void enableWaitForSIFS (void);
	void disableWaitForSIFS (void);

	/* If ACK is enabled, we wait ACKTimeout for an ACK.
	 */
	void enableACK (void);
	void disableACK (void);

	/* If RTS is enabled, we wait CTSTimeout for a CTS.
	 * Otherwise, no RTS is sent.
	 */
	void enableRTS (void);
	void disableRTS (void);

	void setListener (EventListener *listener);

	/* This txmode is applied to _every_ packet sent after
	 * this call. If you want to use a different txMode
	 * for a RTS and its subsequent DATA frame, you need
	 * to call this method twice, once before the start
	 * of the transmission for the RTS and once when the
	 * gotCTS method is called for the DATA.
	 */
	void setTransmissionMode (int txMode);

	void startTransmission (Packet *packet);
	void startBlockAckReqTransmission (int to, int tid);

	void receive (Packet *packet);

	void registerNavListener (NavListener *listener);

private:
	void dropPacket (Packet *packet);
	MacLowParameters *parameters (void);
	double now (void);
	MacStation *lookupStation (int address);
	Phy80211 *peekPhy (void);
	void forwardDown (Packet *packet);
	int getSelf (void);
	double getEIFS (void);
	double getDIFS (void);
	double getLastSNR (void);
	double getLastStartRx (void);
	double calculateTxDuration (int mode, int size);
	void notifyNav (double now, double duration);
	bool isNavZero (double now);

	bool isData (Packet *packet);
	bool isManagement (Packet *packet);
	
	Packet *getRTSPacket (void);
	Packet *getCTSPacket (void);
	Packet *getACKPacket (void);
	Packet *getRTSforPacket (Packet *data);	

	void ACKTimeout (MacCancelableEvent *event);
	void CTSTimeout (MacCancelableEvent *event);
	void sendCTS_AfterRTS (MacCancelableEvent *macEvent);
	void sendACK_AfterData (MacCancelableEvent *macEvent);
	void sendDataAfterCTS (MacCancelableEvent *macEvent);

	void sendRTSForPacket (Packet *packet);
	void sendDataPacket (Packet *packet);
	void sendCurrentTxPacket (void);
	void startTransmission (void);

	/* XXX init all */
	Mac80211 *m_mac;
	MacHigh *m_high;
	Phy80211 *m_phy;

	Packet *m_currentTxPacket;

	EventListener *m_listener;
	vector<NavListener *> m_navListeners;

	DynamicHandler<MacLow> *m_ACKTimeoutHandler;
	DynamicHandler<MacLow> *m_CTSTimeoutHandler;
	DynamicHandler<MacLow> *m_sendCTSHandler;
	DynamicHandler<MacLow> *m_sendACKHandler;
	DynamicHandler<MacLow> *m_sendDataHandler;

	MacLowParameters *m_parameters;


	bool m_waitSIFS;
	bool m_waitACK;
	bool m_sendRTS;
	int m_txMode;
};

#endif /* MAC_LOW_H */
