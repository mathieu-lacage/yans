/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#ifndef MAC_HIGH_NQSTA_H
#define MAC_HIGH_NQSTA_H

#include "mac-address.h"
#include "callback.tcc"
#include "supported-rates.h"
#include <stdint.h>

namespace yans {

class CancellableEvent;
class Packet;
class ChunkMac80211Hdr;
class NetworkInterface80211;
class DcaTxop;
class Timeout;

class MacHighNqsta {
public:
	typedef Callback<void (Packet *)> ForwardCallback;
	typedef Callback<void (void)> AssociatedCallback;

	MacHighNqsta ();
	~MacHighNqsta ();

	void set_dca_txop (DcaTxop *dca);
	void set_interface (NetworkInterface80211 *interface);
	void set_forward_callback (ForwardCallback *callback);
	void set_associated_callback (AssociatedCallback *callback);
	void set_supported_rates (SupportedRates rates);

	void set_max_missed_beacons (uint32_t missed);
	void set_probe_request_timeout (uint64_t us);
	void set_assoc_request_timeout (uint64_t us);

	MacAddress get_bssid (void) const;

	void queue (Packet *packet, MacAddress to);

	void ack_received (ChunkMac80211Hdr const &hdr);
	void receive (Packet *packet, ChunkMac80211Hdr const *hdr);
private:
	void set_bssid (MacAddress bssid);
	MacAddress get_broadcast_bssid (void);
	void send_probe_request (void);
	void send_association_request ();
	void try_to_ensure_associated (void);
	void assoc_request_timeout (void);
	void probe_request_timeout (void);
	bool is_associated (void);
	SupportedRates get_supported_rates (void);
	void missed_beacons (void);
	enum {
		ASSOCIATED,
		WAIT_PROBE_RESP,
		WAIT_ASSOC_RESP,
		BEACON_MISSED,
		REFUSED
	} m_state;
	uint64_t m_probe_request_timeout_us;
	uint64_t m_assoc_request_timeout_us;
	CancellableEvent *m_probe_request_event;
	CancellableEvent *m_assoc_request_event;
	NetworkInterface80211 *m_interface;
	ForwardCallback *m_forward;
	AssociatedCallback *m_associated_callback;
	SupportedRates m_rates;
	DcaTxop *m_dca;
	Timeout *m_timeout;
	MacAddress m_bssid;
};

}; // namespace yans


#endif /* MAC_HIGH_NQSTA_H */
