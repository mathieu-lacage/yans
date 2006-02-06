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
	virtual void startNext (void) = 0;
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

	/* If ACK is enabled, we wait ACKTimeout for an ACK.
	 */
	void enableACK (void);
	void disableACK (void);

	/* If RTS is enabled, we wait CTSTimeout for a CTS.
	 * Otherwise, no RTS is sent.
	 */
	void enableRTS (void);
	void disableRTS (void);

	/* If NextData is enabled, we add the transmission duration
	 * of the nextData to the durationId and we notify the
	 * transmissionListener at the end of the current
	 * transmission + SIFS.
	 */
	void enableNextData (int size);
	void disableNextData (void);

	/* store the transmission mode for the frames to transmit.
	 */
	void setDataTransmissionMode (int txMode);
	void setRtsTransmissionMode (int txMode);

	/* store the data packet to transmit. */
	void setData (Packet *packet);

	/* store the transmission listener. */
	void setTransmissionListener (MacLowTransmissionListener *listener);

	/* returns how much time the transmission of the data 
	 * as currently configured will take.
	 */
	double calculateHandshakeDuration (void);

	/* start the transmission of the currently-stored data. */
	void startTransmission (void);
	
	void setReceptionListener (MacLowReceptionListener *listener);
	void receive (Packet *packet);

	void registerNavListener (MacLowNavListener *listener);
	void startBlockAckReqTransmission (int to, int tid);
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
	double calculateOverallTxTime (void);
	void notifyNav (double now, double duration);
	bool isNavZero (double now);

	bool isData (Packet *packet);
	bool isManagement (Packet *packet);
	bool isCtl (Packet *packet);
	
	Packet *getRTSPacket (void);
	Packet *getCTSPacket (void);
	Packet *getACKPacket (void);
	Packet *getRTSforPacket (Packet *data);	

	void ACKTimeout (MacCancelableEvent *event);
	void CTSTimeout (MacCancelableEvent *event);
	void sendCTS_AfterRTS (MacCancelableEvent *macEvent);
	void sendACK_AfterData (MacCancelableEvent *macEvent);
	void sendDataAfterCTS (MacCancelableEvent *macEvent);
	void waitSIFSAfterEndTx (MacCancelableEvent *macEvent);

	void sendRTSForPacket (void);
	void sendDataPacket (void);
	void sendCurrentTxPacket (void);

	MacContainer *m_container;
	MacLowReceptionListener *m_receptionListener;
	MacLowTransmissionListener *m_transmissionListener;
	vector<MacLowNavListener *> m_navListeners;

	DynamicHandler<MacLow> *m_ACKTimeoutHandler;
	DynamicHandler<MacLow> *m_CTSTimeoutHandler;
	DynamicHandler<MacLow> *m_sendCTSHandler;
	DynamicHandler<MacLow> *m_sendACKHandler;
	DynamicHandler<MacLow> *m_sendDataHandler;
	DynamicHandler<MacLow> *m_waitSIFSHandler;

	Packet *m_currentTxPacket;

	int m_nextSize;
	bool m_waitACK;
	bool m_sendRTS;
	int m_dataTxMode;
	int m_rtsTxMode;
	double m_lastNavStart;
	double m_lastNavDuration;
};

#endif /* MAC_LOW_H */
