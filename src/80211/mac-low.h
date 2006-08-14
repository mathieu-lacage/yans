/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005, 2006 INRIA
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

#include <vector>
#include <stdint.h>

#include "chunk-mac-80211-hdr.h"
#include "mac-address.h"
#include "callback.h"
#include "event.h"
#include "gpacket.h"

namespace yans {

class NetworkInterface80211;
class Phy80211;
class PacketLogger;
class MacStations;
class MacStation;
class MacParameters;

class MacLowTransmissionListener {
public:
	MacLowTransmissionListener ();
	virtual ~MacLowTransmissionListener ();

	virtual void got_cts (double snr, uint8_t tx_mode) = 0;
	virtual void missed_cts (void) = 0;
	/* Do not rely on the gotAck method to be
	 * given valid parameters when SuperFastAck is
	 * enabled.
	 */
	virtual void got_ack (double snr, uint8_t tx_mode) = 0;
	virtual void missed_ack (void) = 0;
	virtual void start_next (void) = 0;

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
	virtual void nav_start_us (uint64_t now_us, uint64_t duration_us) = 0;
	virtual void nav_continue_us (uint64_t now_us, uint64_t duration_us) = 0;
	virtual void nav_reset_us (uint64_t now_us, uint64_t duration_us) = 0;
};

class MacLowTransmissionParameters {
public:
	MacLowTransmissionParameters ();
		
	/* If ACK is enabled, we wait ACKTimeout for an ACK.
	 */
	void enable_ack (void);
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
	void enable_fast_ack (void);
	/* If SuperFastAck is enabled, we:
	 *   - if busy at end-of-tx+PIFS, report gotAck
	 *   - if idle at end-of-tx+PIFS, report missedAck
	 */
	void enable_super_fast_ack (void);
	/* If RTS is enabled, we wait CTSTimeout for a CTS.
	 * Otherwise, no RTS is sent.
	 */
	void enable_rts (void);
	/* If NextData is enabled, we add the transmission duration
	 * of the nextData to the durationId and we notify the
	 * transmissionListener at the end of the current
	 * transmission + SIFS.
	 */
	void enable_next_data (uint32_t size);
	
	/* If we enable this, we ignore all other durationId 
	 * calculation and simply force the packet's durationId
	 * field to this value.
	 */
	void enable_override_duration_id (uint64_t duration_id_us);
	
	void disable_ack (void);
	void disable_rts (void);
	void disable_next_data (void);
	void disable_override_duration_id (void);

	bool must_wait_ack (void) const;
	bool must_wait_normal_ack (void) const;
	bool must_wait_fast_ack (void) const;
	bool must_wait_super_fast_ack (void) const;
	bool must_send_rts (void) const;
	bool has_duration_id (void) const;
	uint64_t get_duration_id (void) const;
	bool has_next_packet (void) const;
	uint32_t get_next_packet_size (void) const;

private:
	uint32_t m_next_size;
	enum {
		ACK_NONE,
		ACK_NORMAL,
		ACK_FAST,
		ACK_SUPER_FAST
	} m_wait_ack;
	bool m_send_rts;
	uint64_t m_override_duration_id_us;
};


class MacLow {
public:
	typedef Callback<void, GPacket , ChunkMac80211Hdr const*> MacLowRxCallback;

	MacLow ();
	~MacLow ();

	void set_interface (NetworkInterface80211 *interface);
	void set_phy (Phy80211 *phy);
	void set_stations (MacStations *stations);
	void set_parameters (MacParameters *parameters);
	void set_rx_callback (MacLowRxCallback callback);
	void register_nav_listener (MacLowNavListener *listener);

	/* This transmission time includes the time required for
	 * the next packet transmission if one was selected.
	 */
	uint64_t calculate_transmission_time (uint32_t payload_size,
					      MacAddress to,
					      MacLowTransmissionParameters const&parameters) const;

	/* start the transmission of the currently-stored data. */
	void start_transmission (GPacket packet, 
				 ChunkMac80211Hdr const*hdr, 
				 MacLowTransmissionParameters parameters,
				 MacLowTransmissionListener *listener);

	void receive_ok (GPacket const packet, double rx_snr, uint8_t tx_mode, uint8_t stuff);
	void receive_error (GPacket const packet, double rx_snr);
private:
	void cancel_all_events (void);
	uint32_t get_ack_size (void) const;
	uint32_t get_rts_size (void) const;
	uint32_t get_cts_size (void) const;
	uint64_t get_sifs_us (void) const;
	uint64_t get_pifs_us (void) const;
	uint64_t get_ack_timeout_us (void) const;
	uint64_t get_cts_timeout_us (void) const;
	uint32_t get_current_size (void) const;
	uint64_t now_us (void) const;
	MacStation *get_station (MacAddress to) const;
	void forward_down (GPacket const packet, ChunkMac80211Hdr const *hdr, 
			   uint8_t tx_mode, uint8_t stuff);
	uint64_t calculate_overall_tx_time_us (uint32_t size,
					       MacAddress to,
					       MacLowTransmissionParameters const &params) const;
	uint8_t get_rts_tx_mode (MacAddress to) const;
	uint8_t get_data_tx_mode (MacAddress to, uint32_t size) const;
	uint8_t get_cts_tx_mode_for_rts (MacAddress to,  uint8_t rts_tx_mode) const;
	uint8_t get_ack_tx_mode_for_data (MacAddress to, uint8_t data_tx_mode) const;
	void notify_nav (uint64_t now_time_us, ChunkMac80211Hdr const*hdr);
	bool is_nav_zero (uint64_t now_time_us);
	void maybe_cancel_previous (void);
	
	void normal_ack_timeout (void);
	void fast_ack_timeout (void);
	void super_fast_ack_timeout (void);
	void fast_ack_failed_timeout (void);
	void cts_timeout (void);
	void send_cts_after_rts (MacAddress source, uint64_t duration_us, uint8_t tx_mode, uint8_t stuff);
	void send_ack_after_data (MacAddress source, uint64_t duration_us, uint8_t tx_mode, uint8_t stuff);
	void send_data_after_cts (MacAddress source, uint64_t duration_us, uint8_t tx_mode);
	void wait_sifs_after_end_tx (void);

	void send_rts_for_packet (void);
	void send_data_packet (void);
	void send_current_tx_packet (void);
	void start_data_tx_timers (void);

	NetworkInterface80211 *m_interface;
	Phy80211 *m_phy;
	MacStations *m_stations;
	MacParameters *m_parameters;
	MacLowRxCallback m_rx_callback;
	typedef std::vector<MacLowNavListener *>::const_iterator NavListenersCI;
	typedef std::vector<MacLowNavListener *> NavListeners;
	NavListeners m_nav_listeners;

	Event m_normal_ack_timeout_event;
	Event m_fast_ack_timeout_event;
	Event m_super_fast_ack_timeout_event;
	Event m_fast_ack_failed_timeout_event;
	Event m_cts_timeout_event;
	Event m_send_cts_event;
	Event m_send_ack_event;
	Event m_send_data_event;
	Event m_wait_sifs_event;

	GPacket m_current_packet;
	bool m_has_current;
	ChunkMac80211Hdr m_current_hdr;
	MacLowTransmissionParameters m_tx_params;
	MacLowTransmissionListener *m_listener;

	uint64_t m_last_nav_start_us;
	uint64_t m_last_nav_duration_us;

	PacketLogger *m_drop_error;
};

}; // namespace yans

#endif /* MAC_LOW_H */
