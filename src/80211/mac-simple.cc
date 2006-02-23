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
#include "mac-simple.h"
#include "phy-80211.h"
#include "mac-station.h"
#include "mac-stations.h"
#include "cancellable-event.tcc"
#include "simulator.h"
#include "packet.h"
#include "chunk-mac-80211-hdr.h"

namespace yans {

MacSimple::MacSimple ()
{
	m_use_rts = false;
	m_rts_retry = 0;
	m_data_retry = 0;
	m_rts_retry_max = 0;
	m_data_retry_max = 0;
	m_current = 0;

	m_rts_timeout_event = 0;
	m_data_timeout_event = 0;

	//XXX
	m_rts_timeout_us = 0;
	m_data_timeout_us = 0;
}

void 
MacSimple::set_phy (Phy80211 *phy)
{
	m_phy = phy;
}
void 
MacSimple::set_stations (MacStations *stations)
{
	m_stations = stations;
}
void 
MacSimple::enable_rts_cts (void)
{
	m_use_rts = true;
}
  
void 
MacSimple::send (Packet *packet)
{
	if (!m_phy->is_state_idle () ||
	    m_current != 0) {
		// busy sending something.
		return;
	}
	m_current = packet;
	m_current->ref ();
	if (m_use_rts) {
		m_rts_retry = 0;
		send_rts ();
	} else {
		m_data_retry = 0;
		send_data ();
	}
}
void 
MacSimple::receive_ok (Packet *packet, double snr, uint8_t tx_mode)
{
	MacStation *station = get_station ();
	station->report_rx_ok (snr, tx_mode);
	ChunkMac80211Hdr hdr;
	packet->remove (&hdr);
	if (hdr.is_rts ()) {
		send_cts (tx_mode);
	} else if (hdr.is_cts ()) {
		station->report_rts_ok (snr, tx_mode);
		send_data ();
	} else if (hdr.is_data ()) {
		send_ack (tx_mode);
	} else if (hdr.is_ack ()) {
		station->report_data_ok (snr, tx_mode);
		m_current->unref ();
		m_current = 0;
	}
}
void 
MacSimple::receive_error (Packet *packet)
{}

void
MacSimple::send_cts (uint8_t tx_mode)
{
	Packet *packet = new Packet ();
	ChunkMac80211Hdr cts;
	cts.set_type (MAC_80211_CTL_CTS);
	packet->add (&cts);
	m_phy->send_packet (packet, tx_mode, 0);
	packet->unref ();
}

void
MacSimple::send_ack (uint8_t tx_mode)
{
	Packet *packet = new Packet ();
	ChunkMac80211Hdr ack;
	ack.set_type (MAC_80211_CTL_ACK);
	packet->add (&ack);
	m_phy->send_packet (packet, tx_mode, 0);
	packet->unref ();
}


void
MacSimple::send_rts (void)
{
	MacStation *station = get_station ();
	Packet *packet = new Packet ();
	ChunkMac80211Hdr rts;
	rts.set_type (MAC_80211_CTL_RTS);
	packet->add (&rts);
	uint64_t tx_duration = m_phy->calculate_tx_duration_us (packet->get_size (), station->get_rts_mode ());
	assert (m_rts_timeout_event == 0);
	m_rts_timeout_event = make_cancellable_event (&MacSimple::retry_rts, this);
	Simulator::insert_in_us (tx_duration + m_rts_timeout_us, m_rts_timeout_event);
	m_phy->send_packet (packet, station->get_rts_mode (), 0);
	packet->unref ();
}

void
MacSimple::send_data (void)
{
	MacStation *station = get_station ();
	Packet *packet = m_current->copy ();
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	packet->add (&hdr);
	uint64_t tx_duration = m_phy->calculate_tx_duration_us (packet->get_size (), station->get_data_mode (packet->get_size ()));
	assert (m_data_timeout_event == 0);
	m_data_timeout_event = make_cancellable_event (&MacSimple::retry_data, this);
	Simulator::insert_in_us (tx_duration + m_data_timeout_us, m_data_timeout_event);
	m_phy->send_packet (packet, station->get_data_mode (packet->get_size ()), 0);
	packet->unref ();
}

MacStation *
MacSimple::get_station (void)
{
	return m_stations->lookup (MacAddress ("ff:ff:ff:ff:ff:ff"));
}

void
MacSimple::retry_data (void)
{
	m_data_timeout_event = 0;
	m_data_retry++;
	MacStation *station = get_station ();
	station->report_data_failed ();
	if (m_data_retry > m_data_retry_max) {
		station->report_final_data_failed ();
		m_current->unref ();
		m_current = 0;
		return;
	}
	m_rts_retry = 0;
	send_rts ();
}

void
MacSimple::retry_rts (void)
{
	m_rts_timeout_event = 0;
	m_rts_retry++;
	MacStation *station = get_station ();
	station->report_rts_failed ();
	if (m_rts_retry > m_rts_retry_max) {
		station->report_final_rts_failed ();
		m_current->unref ();
		m_current = 0;
		return;
	}
	send_rts ();
}


}; // namespace yans
