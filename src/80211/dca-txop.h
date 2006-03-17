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
#include "callback.tcc"
#include "chunk-mac-80211-hdr.h"

namespace yans {

class Dcf;
class MacQueue80211e;
class Packet;
class MacLow;
class MacParameters;
class MacTxMiddle;

class DcaTxop 
{
public:
	typedef Callback <void (ChunkMac80211Hdr const&)> AckReceived;

	DcaTxop ();
	~DcaTxop ();

	void set_dcf (Dcf *dcf);
	void set_queue (MacQueue80211e *queue);
	void set_low (MacLow *low);
	void set_parameters (MacParameters *parameters);
	void set_tx_middle (MacTxMiddle *tx_middle);
	void set_ack_received_callback (AckReceived *callback);

private:
	class AccessListener;
	class TransmissionListener;
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
	Packet *get_fragment_packet (ChunkMac80211Hdr *hdr);

	Dcf *m_dcf;
	AckReceived *m_ack_received;
	MacQueue80211e *m_queue;
	MacTxMiddle *m_tx_middle;
	MacLow *m_low;
	MacParameters *m_parameters;
	TransmissionListener *m_transmission_listener;
	AccessListener *m_access_listener;
	

	Packet *m_current_packet;
	ChunkMac80211Hdr m_current_hdr;
	uint32_t m_ssrc;
	uint32_t m_slrc;
	uint8_t m_fragment_number;
};

}; //namespace yans



#endif /* DCA_TXOP_H */
