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
class MacContainer;
class MacParameters;
class MacStation;

class MacLowTransmissionListener {
public:
	MacLowTransmissionListener ();
	virtual ~MacLowTransmissionListener ();

	virtual void gotCTS (double snr, int txMode) = 0;
	virtual void missedCTS (void) = 0;
	virtual void gotACK (double snr, int txMode) = 0;
	virtual void missedACK (void) = 0;
	virtual void txCompletedAndSIFS (void) = 0;
	virtual void gotBlockAckStart (double snr) = 0;
	virtual void gotBlockAck (int sequence) = 0;
	virtual void gotBlockAckEnd (void) = 0;
	virtual void missedBlockAck (void) = 0;
};

class MacLowReceptionListener {
public:
	MacLowReceptionListener ();
	virtual ~MacLowReceptionListener ();

	virtual void gotPacket (int from, double snr, int txMode) = 0;
	virtual void gotData (Packet *packet) = 0;
	/* return the BlockAckResp. If 0 is returned,
	 * the MacLow will send back a simple ACK to the
	 * originator.
	 */
	virtual Packet *gotBlockAckReq (Packet *packet) = 0;
};

class MacLowNavListener {
public:
	MacLowNavListener ();
	virtual ~MacLowNavListener ();
	virtual void navStart (double now, double duration) = 0;
	virtual void navContinue (double duration) = 0;
};

class MacLow {
public:
	MacLow (MacContainer *container);
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

	void setReceptionListener (MacLowReceptionListener *listener);
	void setTransmissionListener (MacLowTransmissionListener *listener);

	void setAdditionalDuration (double duration);
	double getDataTransmissionDuration (int size);

	void setDataTransmissionMode (int txMode);
	void setRtsTransmissionMode (int txMode);

	void startTransmission (Packet *packet);
	void startBlockAckReqTransmission (int to, int tid);

	void receive (Packet *packet);

	void registerNavListener (MacLowNavListener *listener);

private:
	void dropPacket (Packet *packet);
	MacParameters *parameters (void);
	double now (void);
	MacStation *lookupStation (int address);
	Phy80211 *peekPhy (void);
	void forwardDown (Packet *packet);
	int getSelf (void);
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

	MacContainer *m_container;
	MacLowReceptionListener *m_receptionListener;
	MacLowTransmissionListener *m_transmissionListener;
	vector<MacLowNavListener *> m_navListeners;

	DynamicHandler<MacLow> *m_ACKTimeoutHandler;
	DynamicHandler<MacLow> *m_CTSTimeoutHandler;
	DynamicHandler<MacLow> *m_sendCTSHandler;
	DynamicHandler<MacLow> *m_sendACKHandler;
	DynamicHandler<MacLow> *m_sendDataHandler;

	Packet *m_currentTxPacket;

	bool m_waitSIFS;
	bool m_waitACK;
	bool m_sendRTS;
	int m_dataTxMode;
	int m_rtsTxMode;
	double m_additionalDuration;
	double m_lastNavStart;
	double m_lastNavDuration;
};

#endif /* MAC_LOW_H */
