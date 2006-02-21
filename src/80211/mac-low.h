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

#include "chunk-mac-80211-hdr.h"
#include "mac-address.h"

namespace yans {

class Packet;
class NetInterface80211;
class CancellableEvent;
class PacketLogger;

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
	virtual void navReset (double now, double duration) = 0;
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
	void setData (Packet *packet, ChunkMac80211Hdr const*hdr);

	/* store the transmission listener. */
	void setTransmissionListener (MacLowTransmissionListener *listener);

	/* start the transmission of the currently-stored data. */
	void startTransmission (void);

	void receive_ok (Packet *packet, double rx_snr, int tx_mode);
	void receive_error (Packet *packet);
	
	void registerNavListener (MacLowNavListener *listener);
private:
	uint32_t get_ack_size (void) const;
	uint32_t get_rts_size (void) const;
	uint32_t get_cts_size (void) const;
	double get_sifs (void) const;
	double get_pifs (void) const;
	double get_ack_timeout (void) const;
	double get_cts_timeout (void) const;
	uint32_t get_current_size (void) const;
	double now (void);
	void forward_down (Packet *packet, int tx_mode, ChunkMac80211Hdr const *hdr);
	void forward_up (Packet *packet, ChunkMac80211Hdr const *hdr);
	MacAddress getSelf (void);
	bool waitAck (void);
	bool waitNormalAck (void);
	bool waitFastAck (void);
	bool waitSuperFastAck (void);
	double calculateTxDuration (int mode, uint32_t size);
	double calculateOverallCurrentTxTime (void);
	int getCtsTxModeForRts (int to,  int rtsTxMode);
	int getAckTxModeForData (int to, int dataTxMode);
	void notifyNav (double now, ChunkMac80211Hdr const*hdr);
	bool isNavZero (double now);
	void maybeCancelPrevious (void);
	
	Packet *getRTSPacket (void);
	Packet *getCTSPacket (void);
	Packet *getACKPacket (void);
	Packet *getRTSforPacket (Packet *data);	

	void normal_ack_timeout (void);
	void fast_ack_timeout (void);
	void super_fast_ack_timeout (void);
	void fast_ack_failed_timeout (void);
	void cts_timeout (void);
	void send_cts_after_rts (MacAddress source, double duration, int tx_mode);
	void send_ack_after_data (MacAddress source, double duration, int tx_mode);
	void send_data_after_cts (MacAddress source, double duration, int tx_mode);
	void wait_sifs_after_end_tx (void);

	void send_rts_for_packet (void);
	void send_data_packet (void);
	void sendCurrentTxPacket (void);

	NetInterface80211 *m_interface;
	MacLowTransmissionListener *m_transmissionListener;
	typedef std::vector<MacLowNavListener *>::const_iterator NavListenersCI;
	typedef std::vector<MacLowNavListener *> NavListeners;
	NavListeners m_navListeners;

	CancellableEvent *m_normal_ack_timeout_event;
	CancellableEvent *m_fast_ack_timeout_event;
	CancellableEvent *m_super_fast_ack_timeout_event;
	CancellableEvent *m_fast_ack_failed_timeout_event;
	CancellableEvent *m_cts_timeout_event;
	CancellableEvent *m_send_cts_event;
	CancellableEvent *m_send_ack_event;
	CancellableEvent *m_send_data_event;
	CancellableEvent *m_wait_sifs_event;

	Packet *m_current_packet;
	ChunkMac80211Hdr m_current_hdr;

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

	PacketLogger *m_drop_error;
};

}; // namespace yans

#endif /* MAC_LOW_H */
