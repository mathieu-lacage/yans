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
#ifndef MAC_LOW_H
#define MAC_LOW_H

#include <vector>

#include "hdr-mac-80211.h"
#include "mac-handler.tcc"

class Packet;
class NetInterface80211;

class MacLowTransmissionListener {
public:
	MacLowTransmissionListener ();
	virtual ~MacLowTransmissionListener ();

	virtual void gotCTS (double snr, int txMode) = 0;
	virtual void missedCTS (void) = 0;
	/* Do not rely on the gotAck method to be
	 * given valid parameters when SuperFastAck is
	 * enabled.
	 */
	virtual void gotACK (double snr, int txMode) = 0;
	virtual void missedACK (void) = 0;
	virtual void startNext (void) = 0;

	/* Invoked if this transmission was canceled 
	 * one way or another. When this method is invoked,
	 * you can assume that the packet has not been passed
	 * down the stack to the PHY. You are responsible
	 * for freeing the packet if you want to.
	 */
	virtual void cancel (void) = 0;
};


class MacLowNavListener {
public:
	MacLowNavListener ();
	virtual ~MacLowNavListener ();
	virtual void navStart (double now, double duration) = 0;
	virtual void navContinue (double duration) = 0;
	virtual void navReset (double now) = 0;
};

class MacLow {
public:
	MacLow ();
	~MacLow ();

	void setInterface (NetInterface80211 *interface);


	/* If ACK is enabled, we wait ACKTimeout for an ACK.
	 */
	void enableACK (void);
	/* If FastAck is enabled, we:
	 *   - wait PIFS after end-of-tx. If idle, report
	 *     FastAckMissed.
	 *   - if busy at end-of-tx+PIFS, wait end-of-rx
	 *   - if Ack ok at end-of-rx, report FastAck ok.
	 *   - if Ack not ok at end-of-rx, report FastAckMissed
	 *     at end-of-rx+SIFS.
	 * This is really complicated but it is needed for
	 * proper HCCA support.
	 */
	void enableFastAck (void);
	/* If SuperFastAck is enabled, we:
	 *   - if busy at end-of-tx+PIFS, report gotAck
	 *   - if idle at end-of-tx+PIFS, report missedAck
	 */
	void enableSuperFastAck (void);
	/* disable either Ack or FastAck, depending on
	 * which Ack method was enabled.
	 */
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
	void enableNextData (uint32_t size, int txMode);
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

	/* start the transmission of the currently-stored data. */
	void startTransmission (void);

	void receive (Packet *packet);
	
	void registerNavListener (MacLowNavListener *listener);
private:
	void dropPacket (Packet *packet);
	double now (void);
	void forwardDown (Packet *packet);
	int getSelf (void);
	bool waitAck (void);
	bool waitNormalAck (void);
	bool waitFastAck (void);
	bool waitSuperFastAck (void);
	double getLastSNR (void);
	double calculateTxDuration (int mode, uint32_t size);
	double calculateOverallCurrentTxTime (void);
	int getCtsTxModeForRts (int to,  int rtsTxMode);
	int getAckTxModeForData (int to, int dataTxMode);
	void notifyNav (double now, double duration,
			enum mac_80211_packet_type type,
			int source);
	bool isNavZero (double now);
	void maybeCancelPrevious (void);
	
	Packet *getRTSPacket (void);
	Packet *getCTSPacket (void);
	Packet *getACKPacket (void);
	Packet *getRTSforPacket (Packet *data);	

	void normalAckTimeout (MacCancelableEvent *event);
	void fastAckTimeout (MacCancelableEvent *event);
	void superFastAckTimeout (MacCancelableEvent *event);
	void fastAckFailedTimeout (MacCancelableEvent *event);
	void CTSTimeout (MacCancelableEvent *event);
	void sendCTS_AfterRTS (MacCancelableEvent *macEvent);
	void sendACK_AfterData (MacCancelableEvent *macEvent);
	void sendDataAfterCTS (MacCancelableEvent *macEvent);
	void waitSIFSAfterEndTx (MacCancelableEvent *macEvent);

	void sendRTSForPacket (void);
	void sendDataPacket (void);
	void sendCurrentTxPacket (void);

	NetInterface80211 *m_interface;
	MacLowTransmissionListener *m_transmissionListener;
	typedef std::vector<MacLowNavListener *>::const_iterator NavListenersCI;
	typedef std::vector<MacLowNavListener *> NavListeners;
	NavListeners m_navListeners;

	DynamicHandler<MacLow> *m_normalAckTimeoutHandler;
	DynamicHandler<MacLow> *m_fastAckTimeoutHandler;
	DynamicHandler<MacLow> *m_superFastAckTimeoutHandler;
	DynamicHandler<MacLow> *m_fastAckFailedTimeoutHandler;
	DynamicHandler<MacLow> *m_CTSTimeoutHandler;
	DynamicHandler<MacLow> *m_sendCTSHandler;
	DynamicHandler<MacLow> *m_sendACKHandler;
	DynamicHandler<MacLow> *m_sendDataHandler;
	DynamicHandler<MacLow> *m_waitSIFSHandler;

	Packet *m_currentTxPacket;

	uint32_t m_nextSize;
	int m_nextTxMode;
	enum {
		ACK_NONE,
		ACK_NORMAL,
		ACK_FAST,
		ACK_SUPER_FAST
	} m_waitAck;
	bool m_sendRTS;
	int m_dataTxMode;
	int m_rtsTxMode;
	double m_lastNavStart;
	double m_lastNavDuration;
	double m_overrideDurationId;
};

#endif /* MAC_LOW_H */
