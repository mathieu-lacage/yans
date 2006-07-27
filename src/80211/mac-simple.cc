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
#include "event.tcc"
#include "simulator.h"
#include "packet.h"
#include "chunk-mac-80211-hdr.h"
#include "mac-network-interface.h"
#include <cassert>

#define noMACSIMPLE_DEBUG 1

#ifdef MACSIMPLE_DEBUG
#include <iostream>
#  define TRACE(x) \
std::cout << "MACSIMPLE TRACE " << Simulator::now_us () << "us " << x << std::endl;
#else
#  define TRACE(x)
#endif


namespace yans {

MacSimple::MacSimple ()
{
	m_rts_retry = 0;
	m_data_retry = 0;
	m_rts_retry_max = 3;
	m_data_retry_max = 3;
	m_current = 0;
	m_rts_cts_threshold = 1;

	m_rts_timeout_us = (uint64_t) ((10 /*cts*/ + 2/* padding for prop time*/)* 8 / 3e6 * 1e6) + 80;
	m_data_timeout_us = (uint64_t) ((10 /*ack*/ + 2/* padding for prop time*/)* 8 / 3e6 * 1e6) + 80;
}

MacSimple::~MacSimple ()
{}

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
MacSimple::set_interface (MacNetworkInterface *interface)
{
	m_interface = interface;
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
MacSimple::set_receiver (RxCallback data)
{
	m_data_rx = data;
}
void 
MacSimple::set_rts_cts_threshold (uint32_t size)
{
	m_rts_cts_threshold = size;
}
void 
MacSimple::send (PacketPtr packet, MacAddress to)
{
	if (!m_phy->is_state_idle () ||
	    m_current != 0) {
		if (m_current == 0) {
			m_current = packet;
			m_current_to = to;
			m_rts_retry = 0;
			m_data_retry = 0;
			assert (!m_send_later_event.is_running ());
			m_send_later_event = make_event (&MacSimple::send_later, this);
			Simulator::schedule_rel_us (m_phy->get_delay_until_idle_us (),
						 m_send_later_event);
		}
		// busy sending something.
		//TRACE ("don't send because busy.");
		return;
	}
	m_current = packet;
	m_current_to = to;
	m_rts_retry = 0;
	m_data_retry = 0;
	if (use_rts ()) {
		TRACE ("send rts");
		send_rts ();
	} else {
		TRACE ("send data");
		send_data ();
	}
}
void 
MacSimple::receive_ok (ConstPacketPtr p, double snr, uint8_t tx_mode, uint8_t stuff)
{
	ChunkMac80211Hdr hdr;
	p->peek (&hdr);
	if (hdr.is_rts () && 
	    hdr.get_addr1 () == m_interface->get_mac_address ()) {
		MacStation *station = get_station (hdr.get_addr2 ());
		TRACE ("receive rts from "<<hdr.get_addr2 ());
		station->report_rx_ok (snr, tx_mode);
		send_cts (tx_mode, hdr.get_addr2 (), station->snr_to_snr (snr));
	} else if (hdr.is_cts () &&
		   hdr.get_addr1 () == m_interface->get_mac_address ()) {
		MacStation *station = get_station (m_current_to);
		TRACE ("receive cts from "<<m_current_to);
		station->report_rx_ok (snr, tx_mode);
		station->report_rts_ok (snr, tx_mode, stuff);
		assert (m_rts_timeout_event.is_running ());
		m_rts_timeout_event.cancel ();
		send_data ();
	} else if (hdr.is_data ()) {
		MacStation *station = get_station (hdr.get_addr2 ());
		station->report_rx_ok (snr, tx_mode);
		if (hdr.get_addr1 ().is_broadcast ()) {
			TRACE ("receive broadcast data from " <<hdr.get_addr2 ());
			goto rx_packet;
		} else if (hdr.get_addr1 () == m_interface->get_mac_address ()) {
			TRACE ("receive unicast data from " <<hdr.get_addr2 ());
			send_ack (tx_mode, hdr.get_addr2 (), station->snr_to_snr (snr));
			goto rx_packet;
		}
	} else if (hdr.is_ack () &&
		   hdr.get_addr1 () == m_interface->get_mac_address ()) {
		MacStation *station = get_station (m_current_to);
		TRACE ("receive ack from "<<m_current_to);
		station->report_rx_ok (snr, tx_mode);
		station->report_data_ok (snr, tx_mode, stuff);
		assert (m_data_timeout_event.is_running ());
		m_data_timeout_event.cancel ();
		m_current = 0;
	}
	return;
 rx_packet:
	PacketPtr packet = p->copy ();
	packet->remove (&hdr);
	m_data_rx (packet);
	return;
}
void 
MacSimple::receive_error (ConstPacketPtr packet, double snr)
{
	TRACE ("error packet snr="<<snr);
}

void
MacSimple::send_cts (uint8_t tx_mode, MacAddress to, uint8_t rts_snr)
{
	PacketPtr packet = Packet::create ();
	ChunkMac80211Hdr cts;
	cts.set_type (MAC_80211_CTL_CTS);
	cts.set_addr1 (to);
	packet->add (&cts);
	m_phy->send_packet (packet, tx_mode, 0, rts_snr);
}

void
MacSimple::send_ack (uint8_t tx_mode, MacAddress to, uint8_t data_snr)
{
	PacketPtr packet = Packet::create ();
	ChunkMac80211Hdr ack;
	ack.set_type (MAC_80211_CTL_ACK);
	ack.set_addr1 (to);
	packet->add (&ack);
	m_phy->send_packet (packet, tx_mode, 0, data_snr);
}


void
MacSimple::send_rts (void)
{
	MacStation *station = get_station (m_current_to);
	PacketPtr packet = Packet::create ();
	ChunkMac80211Hdr rts;
	rts.set_type (MAC_80211_CTL_RTS);
	rts.set_addr1 (m_current_to);
	rts.set_addr2 (m_interface->get_mac_address ());
	packet->add (&rts);
	uint64_t tx_duration = m_phy->calculate_tx_duration_us (packet->get_size (), station->get_rts_mode ());
	assert (!m_rts_timeout_event.is_running ());
	m_rts_timeout_event = make_event (&MacSimple::retry_rts, this);
	Simulator::schedule_rel_us (tx_duration + get_rts_timeout_us (), m_rts_timeout_event);
	m_phy->send_packet (packet, station->get_rts_mode (), 0, 0);
}

void
MacSimple::send_data (void)
{
	MacStation *station = get_station (m_current_to);
	PacketPtr packet = m_current->copy ();
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	hdr.set_addr1 (m_current_to);
	hdr.set_addr2 (m_interface->get_mac_address ());
	packet->add (&hdr);
	if (m_current_to.is_broadcast ()) {
		m_current = 0;
	} else {
		uint64_t tx_duration = m_phy->calculate_tx_duration_us (packet->get_size (), 
									station->get_data_mode (packet->get_size ()));
		m_data_timeout_event = make_event (&MacSimple::retry_data, this);
		Simulator::schedule_rel_us (tx_duration + get_data_timeout_us (), 
					 m_data_timeout_event);
	}
	m_phy->send_packet (packet, station->get_data_mode (packet->get_size ()), 0, 0);
}

MacStation *
MacSimple::get_station (MacAddress ad)
{
	return m_stations->lookup (ad);
}

void
MacSimple::retry_data (void)
{
	m_data_retry++;
	MacStation *station = get_station (m_current_to);
	station->report_data_failed ();
	if (m_data_retry > m_data_retry_max) {
		assert (m_current != 0);
		m_current = 0;
		TRACE ("stop data retry");
		return;
	}
	TRACE ("retry data");
	send_if_we_can ();
}

void
MacSimple::retry_rts (void)
{
	m_rts_retry++;
	MacStation *station = get_station (m_current_to);
	station->report_rts_failed ();
	if (m_rts_retry > m_rts_retry_max) {
		assert (m_current != 0);
		m_current = 0;
		TRACE ("stop rts retry");
		return;
	}
	TRACE ("retry rts");
	send_if_we_can ();
}

void
MacSimple::send_later (void)
{
	send_if_we_can ();
}

bool 
MacSimple::use_rts (void)
{
	if (m_rts_cts_threshold < m_current->get_size () &&
	    !m_current_to.is_broadcast ()) {
		return true;
	} else {
		return false;
	}
}

void 
MacSimple::send_if_we_can (void)
{
	if (!m_phy->is_state_idle ()) {
		TRACE ("send later again");
		assert (m_current != 0);
		if (!m_send_later_event.is_running ()) {
			m_send_later_event = make_event (&MacSimple::send_later, this);
			Simulator::schedule_rel_us (m_phy->get_delay_until_idle_us (),
						 m_send_later_event);
		}
		return;
	}
	if (use_rts ()) {
		TRACE ("send rts");
		send_rts ();
	} else {
		TRACE ("send data");
		send_data ();
	}	
}


}; // namespace yans
