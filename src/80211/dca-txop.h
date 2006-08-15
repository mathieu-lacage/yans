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

#ifndef DCA_TXOP_H
#define DCA_TXOP_H

#include <stdint.h>
#include "callback.h"
#include "chunk-mac-80211-hdr.h"
#include "gpacket.h"
#include "callback-logger.h"

namespace yans {

class Dcf;
class MacQueue80211e;
class MacLow;
class Phy80211;
class MacParameters;
class MacTxMiddle;
class TraceContainer;

class DcaTxop 
{
public:
	typedef Callback <void, ChunkMac80211Hdr const&> TxOk;
	typedef Callback <void, ChunkMac80211Hdr const&> TxFailed;

	DcaTxop ();
	~DcaTxop ();

	void set_low (MacLow *low);
	void set_phy (Phy80211 *phy);
	void set_parameters (MacParameters *parameters);
	void set_tx_middle (MacTxMiddle *tx_middle);
	void set_tx_ok_callback (TxOk callback);
	void set_tx_failed_callback (TxFailed callback);

	void set_difs_us (uint64_t difs_us);
	void set_eifs_us (uint64_t eifs_us);
	void set_cw_bounds (uint32_t min, uint32_t max);
	void set_max_queue_size (uint32_t size);
	void set_max_queue_delay_us (uint64_t us);

	void register_traces (TraceContainer *container);

	void queue (Packet packet, ChunkMac80211Hdr const &hdr);
private:
	class AccessListener;
	class TransmissionListener;
	class NavListener;
	class PhyListener;
	friend class AccessListener;
	friend class TransmissionListener;

	MacLow *low (void);
	MacParameters *parameters (void);

	/* event handlers */
	void access_granted_now (void);
	bool accessing_and_will_notify (void);
	bool access_needed (void);
	void got_cts (double snr, uint8_t txMode);
	void missed_cts (void);
	void got_ack (double snr, uint8_t txMode);
	void missed_ack (void);
	void start_next (void);

	bool need_rts (void);
	bool need_fragmentation (void);
	uint32_t get_n_fragments (void);
	uint32_t get_last_fragment_size (void);
	uint32_t get_next_fragment_size (void);
	uint32_t get_fragment_size (void);
	bool is_last_fragment (void);
	void next_fragment (void);
	Packet get_fragment_packet (ChunkMac80211Hdr *hdr);

	Dcf *m_dcf;
	TxOk m_tx_ok_callback;
	TxFailed m_tx_failed_callback;
	MacQueue80211e *m_queue;
	MacTxMiddle *m_tx_middle;
	MacLow *m_low;
	MacParameters *m_parameters;
	TransmissionListener *m_transmission_listener;
	AccessListener *m_access_listener;
	NavListener *m_nav_listener;
	PhyListener *m_phy_listener;
	

	Packet m_current_packet;
	bool m_has_current;
	ChunkMac80211Hdr m_current_hdr;
	uint32_t m_ssrc;
	uint32_t m_slrc;
	uint8_t m_fragment_number;

	/* 80211-dca-acktimeout
	 * param1: slrc
	 */
	CallbackLogger<uint32_t> m_acktimeout_trace;
	/* 80211-dca-ctstimeout
	 * param1: ssrc
	 */
	CallbackLogger<uint32_t> m_ctstimeout_trace;
};

}; //namespace yans



#endif /* DCA_TXOP_H */
