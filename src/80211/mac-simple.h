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
#ifndef MAC_SIMPLE_H
#define MAC_SIMPLE_H

#include <stdint.h>
#include "cancellable-event.tcc"
#include "mac-address.h"
#include "callback.tcc"

namespace yans {

class Phy80211;
class MacStations;
class Packet;
class MacStation;
class NetworkInterface;

class MacSimple {
public:
	typedef Callback<void (Packet *)> RxCallback;

	MacSimple ();
	~MacSimple ();
	void set_phy (Phy80211 *phy);
	void set_stations (MacStations *stations);
	void set_interface (NetworkInterface *interface);
	void enable_rts_cts (void);
	void set_receiver (RxCallback *data);

	void send (Packet *packet, MacAddress to);
	void receive_ok (Packet *packet, double snr, uint8_t tx_mode, uint8_t stuff);
	void receive_error (Packet *packet);
private:
	void send_cts (uint8_t tx_mode, MacAddress to);
	void send_ack (uint8_t tx_mode, MacAddress to);
	void send_rts (void);
	void send_data (void);
	MacStation *get_station (MacAddress ad);
	void retry_data (void);
	void retry_rts (void);
	void send_later (void);
	void send_if_we_can (void);
	uint64_t get_rts_timeout_us (void);
	uint64_t get_data_timeout_us (void);

	Phy80211 *m_phy;
	MacStations *m_stations;
	NetworkInterface *m_interface;
	bool m_use_rts;
	uint32_t m_rts_retry;
	uint32_t m_data_retry;
	uint32_t m_rts_retry_max;
	uint32_t m_data_retry_max;
	uint64_t m_rts_timeout_us;
	uint64_t m_data_timeout_us;
	Packet *m_current;
	MacAddress m_current_to;
	CancellableEvent *m_rts_timeout_event;
	CancellableEvent *m_data_timeout_event;
	Event *m_send_later_event;
	RxCallback *m_data_rx;
};

}; // namespace yans



#endif /* MAC_SIMPLE_H */
