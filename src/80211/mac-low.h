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
};

class NullMacLowTransmissionListener : public MacLowTransmissionListener {
public:
	NullMacLowTransmissionListener ();
	virtual ~NullMacLowTransmissionListener ();

	virtual void gotCTS (double snr, int txMode);
	virtual void missedCTS (void);
	virtual void gotACK (double snr, int txMode);
	virtual void missedACK (void);
	virtual void startNext (void);

	static MacLowTransmissionListener *instance (void);
private:
	static MacLowTransmissionListener *m_instance;
};

class MacLowReceptionListener {
public:
	MacLowReceptionListener ();
	virtual ~MacLowReceptionListener ();

	virtual void gotPacket (int from, double snr, int txMode) = 0;
	virtual void gotData (Packet *packet) = 0;
	// XXX should implement this.
	// virtual void missedData (Packet *packet) = 0;
};

class MacLowBusyMonitoringListener {
public:
	MacLowBusyMonitoringListener ();
	virtual ~MacLowBusyMonitoringListener ();
	virtual void gotBusyTimeout (void) = 0;
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

	/* If we enable this, we ignore all other durationId 
	 * calculation and simply force the packet's durationId
	 * field to this value.
	 */
	void enableOverrideDurationId (double durationId);
	void disableOverrideDurationId (void);

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


	void receive (Packet *packet);
	
	void setReceptionListener (MacLowReceptionListener *listener);


	/* If "Busy Monitoring" is enabled, the MacLow notifies
	 * the ReceptionListeners whenever the medium stays IDLE
	 * longer than PIFS.
	 * "Busy Monitoring" starts after the next transmission
	 * or reception which happens after calling this method.
	 */
	void enableBusyMonitoring (void);
	void disableBusyMonitoring (void);

	void setBusyMonitoringListener (MacLowBusyMonitoringListener *listener);



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
	double calculateOverallTxTime (void);
	int getCtsTxModeForRts (int to,  int rtsTxMode);
	int getAckTxModeForData (int to, int dataTxMode);
	void notifyNav (double now, double duration);
	bool isNavZero (double now);
	
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
	void busyMonitor (MacCancelableEvent *macEvent);

	void sendRTSForPacket (void);
	void sendDataPacket (void);
	void sendCurrentTxPacket (void);

	MacContainer *m_container;
	MacLowReceptionListener *m_receptionListener;
	MacLowTransmissionListener *m_transmissionListener;
	vector<MacLowNavListener *> m_navListeners;
	MacLowBusyMonitoringListener *m_monitoringListener;

	DynamicHandler<MacLow> *m_ACKTimeoutHandler;
	DynamicHandler<MacLow> *m_CTSTimeoutHandler;
	DynamicHandler<MacLow> *m_sendCTSHandler;
	DynamicHandler<MacLow> *m_sendACKHandler;
	DynamicHandler<MacLow> *m_sendDataHandler;
	DynamicHandler<MacLow> *m_waitSIFSHandler;
	DynamicHandler<MacLow> *m_busyMonitorHandler;

	Packet *m_currentTxPacket;

	int m_nextSize;
	bool m_waitACK;
	bool m_monitorBusy;
	bool m_sendRTS;
	int m_dataTxMode;
	int m_rtsTxMode;
	double m_lastNavStart;
	double m_lastNavDuration;
	double m_overrideDurationId;
};

#endif /* MAC_LOW_H */
