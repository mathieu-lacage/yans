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
#include "network-interface.h"

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

	m_rts_timeout_us = (uint64_t) ((10 /*cts*/ + 2/* padding for prop time*/)* 8 / 3e6 * 1e6);
	m_data_timeout_us = (uint64_t) ((10 /*ack*/ + 2/* padding for prop time*/)* 8 / 3e6 * 1e6);
}

uint64_t 
MacSimple::get_rts_timeout_us (void)
{
	return m_rts_timeout_us;
}
uint64_t 
MacSimple::get_data_timeout_us (void)
{
	return m_data_timeout_us;
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
MacSimple::set_receiver (RxCallback *data)
{
	m_data_rx = data;
}
void 
MacSimple::enable_rts_cts (void)
{
	m_use_rts = true;
}
  
void 
MacSimple::send (Packet *packet, MacAddress to)
{
	if (!m_phy->is_state_idle () ||
	    m_current != 0) {
		// busy sending something.
		return;
	}
	m_current = packet;
	m_current_to = to;
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

	ChunkMac80211Hdr hdr;
	packet->remove (&hdr);
	if (hdr.is_rts ()) {
		MacStation *station = get_station (hdr.get_addr2 ());
		station->report_rx_ok (snr, tx_mode);
		send_cts (tx_mode, hdr.get_addr2 ());
	} else if (hdr.is_cts ()) {
		MacStation *station = get_station (m_current_to);
		station->report_rx_ok (snr, tx_mode);
		station->report_rts_ok (snr, tx_mode);
		send_data ();
	} else if (hdr.is_data ()) {
		MacStation *station = get_station (hdr.get_addr2 ());
		station->report_rx_ok (snr, tx_mode);
		send_ack (tx_mode, hdr.get_addr2 ());
		(*m_data_rx) (packet);
	} else if (hdr.is_ack ()) {
		MacStation *station = get_station (m_current_to);
		station->report_rx_ok (snr, tx_mode);
		station->report_data_ok (snr, tx_mode);
		m_current->unref ();
		m_current = 0;
	}
}
void 
MacSimple::receive_error (Packet *packet)
{}

void
MacSimple::send_cts (uint8_t tx_mode, MacAddress to)
{
	Packet *packet = new Packet ();
	ChunkMac80211Hdr cts;
	cts.set_type (MAC_80211_CTL_CTS);
	cts.set_addr1 (to);
	packet->add (&cts);
	m_phy->send_packet (packet, tx_mode, 0);
	packet->unref ();
}

void
MacSimple::send_ack (uint8_t tx_mode, MacAddress to)
{
	Packet *packet = new Packet ();
	ChunkMac80211Hdr ack;
	ack.set_type (MAC_80211_CTL_ACK);
	ack.set_addr1 (to);
	packet->add (&ack);
	m_phy->send_packet (packet, tx_mode, 0);
	packet->unref ();
}


void
MacSimple::send_rts (void)
{
	MacStation *station = get_station (m_current_to);
	Packet *packet = new Packet ();
	ChunkMac80211Hdr rts;
	rts.set_type (MAC_80211_CTL_RTS);
	rts.set_addr1 (m_current_to);
	rts.set_addr2 (m_interface->get_mac_address ());
	packet->add (&rts);
	uint64_t tx_duration = m_phy->calculate_tx_duration_us (packet->get_size (), station->get_rts_mode ());
	assert (m_rts_timeout_event == 0);
	m_rts_timeout_event = make_cancellable_event (&MacSimple::retry_rts, this);
	Simulator::insert_in_us (tx_duration + get_rts_timeout_us (), m_rts_timeout_event);
	m_phy->send_packet (packet, station->get_rts_mode (), 0);
	packet->unref ();
}

void
MacSimple::send_data (void)
{
	MacStation *station = get_station (m_current_to);
	Packet *packet = m_current->copy ();
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	hdr.set_addr1 (m_current_to);
	hdr.set_addr2 (m_interface->get_mac_address ());
	packet->add (&hdr);
	uint64_t tx_duration = m_phy->calculate_tx_duration_us (packet->get_size (), station->get_data_mode (packet->get_size ()));
	assert (m_data_timeout_event == 0);
	m_data_timeout_event = make_cancellable_event (&MacSimple::retry_data, this);
	Simulator::insert_in_us (tx_duration + get_data_timeout_us (), m_data_timeout_event);
	m_phy->send_packet (packet, station->get_data_mode (packet->get_size ()), 0);
	packet->unref ();
}

MacStation *
MacSimple::get_station (MacAddress ad)
{
	return m_stations->lookup (ad);
}

void
MacSimple::retry_data (void)
{
	m_data_timeout_event = 0;
	m_data_retry++;
	MacStation *station = get_station (m_current_to);
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
	MacStation *station = get_station (m_current_to);
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
