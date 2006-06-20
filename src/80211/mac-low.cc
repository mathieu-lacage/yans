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

#include "mac-low.h"
#include "packet.h"
#include "phy-80211.h"
#include "simulator.h"
#include "chunk-mac-80211-fcs.h"
#include "event.tcc"
#include "network-interface-80211.h"
#include "packet-logger.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "mac-parameters.h"

#include <cassert>

#define noMAC_LOW_TRACE 1

#ifdef MAC_LOW_TRACE
#  include <iostream>
#  define TRACE(x) \
  std::cout << "MAC LOW " << x << std::endl;
#else /* MAC_LOW_TRACE */
# define TRACE(x)
#endif /* MAC_LOW_TRACE */

namespace yans {

MacLowTransmissionListener::MacLowTransmissionListener ()
{}
MacLowTransmissionListener::~MacLowTransmissionListener ()
{}
MacLowNavListener::MacLowNavListener ()
{}
MacLowNavListener::~MacLowNavListener ()
{}

MacLowTransmissionParameters::MacLowTransmissionParameters ()
	: m_next_size (0),
	  m_wait_ack (ACK_NONE),
	  m_send_rts (false),
	  m_override_duration_id_us (0)
{}
void 
MacLowTransmissionParameters::enable_next_data (uint32_t size)
{
	m_next_size = size;
}
void 
MacLowTransmissionParameters::disable_next_data (void)
{
	m_next_size = 0;
}
void 
MacLowTransmissionParameters::enable_override_duration_id (uint64_t duration_id_us)
{
	m_override_duration_id_us = duration_id_us;
}
void 
MacLowTransmissionParameters::disable_override_duration_id (void)
{
	m_override_duration_id_us = 0;
}
void 
MacLowTransmissionParameters::enable_super_fast_ack (void)
{
	m_wait_ack = ACK_SUPER_FAST;
}
void 
MacLowTransmissionParameters::enable_fast_ack (void)
{
	m_wait_ack = ACK_FAST;
}
void 
MacLowTransmissionParameters::enable_ack (void)
{
	m_wait_ack = ACK_NORMAL;
}
void 
MacLowTransmissionParameters::disable_ack (void)
{
	m_wait_ack = ACK_NONE;
}
void 
MacLowTransmissionParameters::enable_rts (void)
{
	m_send_rts = true;
}
void 
MacLowTransmissionParameters::disable_rts (void)
{
	m_send_rts = false;
}
bool 
MacLowTransmissionParameters::must_wait_ack (void) const
{
	return (m_wait_ack != ACK_NONE)?true:false;
}
bool 
MacLowTransmissionParameters::must_wait_normal_ack (void) const
{
	return (m_wait_ack == ACK_NORMAL)?true:false;
}
bool 
MacLowTransmissionParameters::must_wait_fast_ack (void) const
{
	return (m_wait_ack == ACK_FAST)?true:false;
}
bool 
MacLowTransmissionParameters::must_wait_super_fast_ack (void) const
{
	return (m_wait_ack == ACK_SUPER_FAST)?true:false;
}
bool 
MacLowTransmissionParameters::must_send_rts (void) const
{
	return m_send_rts;
}
bool 
MacLowTransmissionParameters::has_duration_id (void) const
{
	return (m_override_duration_id_us != 0)?true:false;
}
uint64_t 
MacLowTransmissionParameters::get_duration_id (void) const
{
	assert (m_override_duration_id_us != 0);
	return m_override_duration_id_us;
}
bool 
MacLowTransmissionParameters::has_next_packet (void) const
{
	return (m_next_size != 0)?true:false;
}
uint32_t 
MacLowTransmissionParameters::get_next_packet_size (void) const
{
	assert (has_next_packet ());
	return m_next_size;
}



MacLow::MacLow ()
	: m_normal_ack_timeout_event (),
	  m_fast_ack_timeout_event (),
	  m_super_fast_ack_timeout_event (),
	  m_fast_ack_failed_timeout_event (),
	  m_cts_timeout_event (),
	  m_send_cts_event (),
	  m_send_ack_event (),
	  m_send_data_event (),
	  m_wait_sifs_event (),
	  m_current_packet ()
{
	m_last_nav_duration_us = 0;
	m_last_nav_start_us = 0;
	m_drop_error = new PacketLogger ();
}

MacLow::~MacLow ()
{
	cancel_all_events ();
	delete m_drop_error;
}

void
MacLow::cancel_all_events (void)
{
	bool one_running = false;
	if (m_normal_ack_timeout_event.is_running ()) {
		m_normal_ack_timeout_event.cancel ();
		one_running = true;
	}
	if (m_fast_ack_timeout_event.is_running ()) {
		m_fast_ack_timeout_event.cancel ();
		one_running = true;
	}
	if (m_super_fast_ack_timeout_event.is_running ()) {
		m_super_fast_ack_timeout_event.cancel ();
		one_running = true;
	}
	if (m_fast_ack_failed_timeout_event.is_running ()) {
		m_fast_ack_failed_timeout_event.cancel ();
		one_running = true;
	}
	if (m_cts_timeout_event.is_running ()) {
		m_cts_timeout_event.cancel ();
		one_running = true;
	}
	if (m_send_cts_event.is_running ()) {
		m_send_cts_event.cancel ();
		one_running = true;
	}
	if (m_send_ack_event.is_running ()) {
		m_send_ack_event.cancel ();
		one_running = true;
	}
	if (m_send_data_event.is_running ()) {
		m_send_data_event.cancel ();
		one_running = true;
	}
	if (m_wait_sifs_event.is_running ()) {
		m_wait_sifs_event.cancel ();
		one_running = true;
	}
	if (one_running && m_listener != 0) {
		m_listener->cancel ();
	}
}

/****************************************************************************
 *        API methods below.
 ****************************************************************************/

void
MacLow::set_interface (NetworkInterface80211 *interface)
{
	m_interface = interface;
}
void
MacLow::set_phy (Phy80211 *phy)
{
	m_phy = phy;
}
void 
MacLow::set_parameters (MacParameters *parameters)
{
	m_parameters = parameters;
}
void 
MacLow::set_stations (MacStations *stations)
{
	m_stations = stations;
}
void 
MacLow::set_rx_callback (MacLowRxCallback callback)
{
	m_rx_callback = callback;
}
void 
MacLow::register_nav_listener (MacLowNavListener *listener)
{
	m_nav_listeners.push_back (listener);
}


void 
MacLow::start_transmission (Packet *packet, 
			    ChunkMac80211Hdr const*hdr, 
			    MacLowTransmissionParameters parameters,
			    MacLowTransmissionListener *listener)
{
	/* m_current_packet is not NULL because someone started
	 * a transmission and was interrupted before one of:
	 *   - ctsTimeout
	 *   - sendDataAfterCTS
	 * expired. This means that one of these timers is still
	 * running. They are all cancelled below anyway by the 
	 * call to cancel_all_events (because of at least one
	 * of these two timer) which will trigger a call to the
	 * previous listener's cancel method.
	 *
	 * This typically happens because the high-priority 
	 * QapScheduler has taken access to the channel from
	 * one of the Edca of the QAP.
	 */
	if (m_current_packet != 0) {
		m_current_packet->unref ();
		m_current_packet = 0;
	}
	m_current_packet = packet;
	m_current_packet->ref ();
	m_current_hdr = *hdr;
	cancel_all_events ();
	m_listener = listener;
	m_tx_params = parameters;

	assert (m_phy->is_state_idle ());

	TRACE ("startTx size="<< get_current_size () << ", to=" << m_current_hdr.get_addr1());

	if (m_tx_params.must_send_rts ()) {
		send_rts_for_packet ();
	} else {
		send_data_packet ();
	}

	/* When this method completes, we have taken ownership of the medium. */
	assert (m_phy->is_state_tx ());	
}

void
MacLow::receive_error (Packet const*packet, double rx_snr)
{
	TRACE ("rx failed ");
	m_drop_error->log (packet);
	if (m_tx_params.must_wait_fast_ack ()) {
		assert (!m_fast_ack_failed_timeout_event.is_running ());
		m_fast_ack_failed_timeout_event = make_event (&MacLow::fast_ack_failed_timeout, this);
		Simulator::insert_in_us (get_sifs_us (), m_fast_ack_failed_timeout_event);
	}
	return;
}

void 
MacLow::receive_ok (Packet const*p, double rx_snr, uint8_t tx_mode, uint8_t stuff)
{
	Packet *packet = p->copy ();
	/* A packet is received from the PHY.
	 * When we have handled this packet,
	 * we handle any packet present in the
	 * packet queue.
	 */
	ChunkMac80211Hdr hdr;
	packet->remove (&hdr);
	ChunkMac80211Fcs fcs;
	packet->remove (&fcs);
	
	bool is_prev_nav_zero = is_nav_zero (now_us ());
	TRACE ("duration/id=" << hdr.get_duration ());
	notify_nav (now_us (), &hdr);
	if (hdr.is_rts ()) {
		/* XXX see section 9.9.2.2.1 802.11e/D12.1 */
		if (is_prev_nav_zero &&
		    hdr.get_addr1 () == m_interface->get_mac_address ()) {
			TRACE ("rx RTS from=" << hdr.get_addr2 () << ", schedule CTS");
			assert (!m_send_cts_event.is_running ());
			MacStation *station = m_stations->lookup (hdr.get_addr2 ());
			station->report_rx_ok (rx_snr, tx_mode);
			m_send_cts_event = make_event (&MacLow::send_cts_after_rts, this,
						       hdr.get_addr2 (), 
						       hdr.get_duration_us (),
						       get_cts_tx_mode_for_rts (hdr.get_addr2 (), tx_mode),
						       station->snr_to_snr (rx_snr));
			Simulator::insert_in_us (get_sifs_us (), m_send_cts_event);
		} else {
			TRACE ("rx RTS from=" << hdr.get_addr2 () << ", cannot schedule CTS");
		}
	} else if (hdr.is_cts () &&
		   hdr.get_addr1 () == m_interface->get_mac_address () &&
		   m_cts_timeout_event.is_running () &&
		   m_current_packet) {
		TRACE ("receive cts from="<<m_current_hdr.get_addr1 ());
		MacStation *station = get_station (m_current_hdr.get_addr1 ());
		station->report_rx_ok (rx_snr, tx_mode);
		station->report_rts_ok (rx_snr, tx_mode, stuff);

		m_cts_timeout_event.cancel ();
		m_listener->got_cts (rx_snr, tx_mode);
		assert (!m_send_data_event.is_running ());
		m_send_data_event = make_event (&MacLow::send_data_after_cts, this, 
						hdr.get_addr1 (),
						hdr.get_duration_us (),
						tx_mode);
		Simulator::insert_in_us (get_sifs_us (), m_send_data_event);
	} else if (hdr.is_ack () &&
		   hdr.get_addr1 () == m_interface->get_mac_address () &&
		   (m_normal_ack_timeout_event.is_running () || 
		    m_fast_ack_timeout_event.is_running () ||
		    m_super_fast_ack_timeout_event.is_running ()) &&
		   m_tx_params.must_wait_ack ()) {
		MacStation *station = get_station (m_current_hdr.get_addr1 ());
		TRACE ("receive ack from="<<m_current_hdr.get_addr1 ());
		station->report_rx_ok (rx_snr, tx_mode);
		station->report_data_ok (rx_snr, tx_mode, stuff);
		bool got_ack = false;
		if (m_tx_params.must_wait_normal_ack () &&
		    m_normal_ack_timeout_event.is_running ()) {
			m_normal_ack_timeout_event.cancel ();
			got_ack = true;
		}
		if (m_tx_params.must_wait_fast_ack () &&
		    m_fast_ack_timeout_event.is_running ()) {
			m_fast_ack_timeout_event.cancel ();
			got_ack = true;
		}
		if (got_ack) {
			m_listener->got_ack (rx_snr, tx_mode);
		}
		if (m_tx_params.has_next_packet ()) {
			m_wait_sifs_event = make_event (&MacLow::wait_sifs_after_end_tx, this);
			Simulator::insert_in_us (get_sifs_us (), m_wait_sifs_event);
		}
	} else if (hdr.is_ctl ()) {
		TRACE ("rx drop " << hdr.get_type_string ());
	} else if (hdr.get_addr1 () == m_interface->get_mac_address ()) {
		MacStation *station = get_station (hdr.get_addr2 ());
		station->report_rx_ok (rx_snr, tx_mode);

		if (hdr.is_qos_data () && hdr.is_qos_no_ack ()) {
			TRACE ("rx unicast/no_ack from="<<hdr.get_addr2 ());
		} else if (hdr.is_data () || hdr.is_mgt ()) {
			TRACE ("rx unicast/send_ack from=" << hdr.get_addr2 ());
			assert (!m_send_ack_event.is_running ());
			m_send_ack_event = make_event (&MacLow::send_ack_after_data, this,
						       hdr.get_addr2 (), 
						       hdr.get_duration_us (),
						       get_ack_tx_mode_for_data (hdr.get_addr2 (), tx_mode),
						       station->snr_to_snr (rx_snr));
			Simulator::insert_in_us (get_sifs_us (), m_send_ack_event);
		}
		m_rx_callback (packet, &hdr);
	} else if (hdr.get_addr1 ().is_broadcast ()) {
		if (hdr.is_data () || hdr.is_mgt ()) {
			TRACE ("rx broadcast from=" << hdr.get_addr2 ());
			m_rx_callback (packet, &hdr);
		} else {
			// DROP.
		}
	} else {
		//TRACE_VERBOSE ("rx not-for-me from %d", getSource (packet));
	}
	packet->unref ();
	return;
}

uint32_t 
MacLow::get_ack_size (void) const
{
	ChunkMac80211Hdr ack;
	ack.set_type (MAC_80211_CTL_ACK);
	return ack.get_size ();
}
uint32_t 
MacLow::get_rts_size (void) const
{
	ChunkMac80211Hdr rts;
	rts.set_type (MAC_80211_CTL_RTS);
	return rts.get_size ();
}
uint32_t 
MacLow::get_cts_size (void) const
{
	ChunkMac80211Hdr cts;
	cts.set_type (MAC_80211_CTL_CTS);
	return cts.get_size ();
}
uint64_t
MacLow::get_sifs_us (void) const
{
	return m_parameters->get_sifs_us ();
}
uint64_t
MacLow::get_pifs_us (void) const
{
	return m_parameters->get_pifs_us ();
}
uint64_t
MacLow::get_ack_timeout_us (void) const
{
	return m_parameters->get_ack_timeout_us ();
}
uint64_t
MacLow::get_cts_timeout_us (void) const
{
	return m_parameters->get_cts_timeout_us ();
}
uint32_t 
MacLow::get_current_size (void) const
{
	ChunkMac80211Fcs fcs;
	return m_current_packet->get_size () + m_current_hdr.get_size () + fcs.get_size ();
}

uint64_t 
MacLow::now_us (void) const
{
	return Simulator::now_us ();
}

uint8_t 
MacLow::get_rts_tx_mode (MacAddress to) const
{
	return get_station (to)->get_rts_mode ();
}
uint8_t 
MacLow::get_data_tx_mode (MacAddress to, uint32_t size) const
{
	return get_station (to)->get_data_mode (size);
}

uint8_t
MacLow::get_cts_tx_mode_for_rts (MacAddress to, uint8_t rts_tx_mode) const
{
	return rts_tx_mode;
}
uint8_t
MacLow::get_ack_tx_mode_for_data (MacAddress to, uint8_t data_tx_mode) const
{
	/* XXX: clearly, this is not very efficient: 
	 * we should be using the min of this mode 
	 * and the BSS mode.
	 */
	return 0;
}


uint64_t
MacLow::calculate_overall_tx_time_us (uint32_t data_size, MacAddress to, 
				      MacLowTransmissionParameters const& params) const
{
	uint64_t tx_time_us = 0;
	MacStation *station = get_station (to);
	uint8_t rts_mode = station->get_rts_mode ();
	uint8_t data_mode = station->get_data_mode (data_size);
	if (params.must_send_rts ()) {
		tx_time_us += m_phy->calculate_tx_duration_us (get_rts_size (), rts_mode);
		uint8_t cts_mode = get_cts_tx_mode_for_rts (m_current_hdr.get_addr1 (), rts_mode);
		tx_time_us += m_phy->calculate_tx_duration_us (get_cts_size (), cts_mode);
		tx_time_us += get_sifs_us () * 2;
	}
	tx_time_us += m_phy->calculate_tx_duration_us (data_size, data_mode);
	if (params.must_wait_ack ()) {
		int ack_mode = get_ack_tx_mode_for_data (m_current_hdr.get_addr1 (), data_mode);
		tx_time_us += get_sifs_us ();
		tx_time_us += m_phy->calculate_tx_duration_us (get_ack_size (), ack_mode);
	}
	return tx_time_us;
}

uint64_t
MacLow::calculate_transmission_time (uint32_t data_size, MacAddress to, 
				     MacLowTransmissionParameters const& params) const
{
	uint64_t tx_time_us = calculate_overall_tx_time_us (data_size, to, params);
	if (params.has_next_packet ()) {
		uint8_t data_mode = get_data_tx_mode (to, data_size );
		tx_time_us += get_sifs_us ();
		tx_time_us += m_phy->calculate_tx_duration_us (params.get_next_packet_size (), data_mode);
	}
	return tx_time_us;
}


void
MacLow::notify_nav (uint64_t now_time_us, ChunkMac80211Hdr const *hdr)
{
	/* XXX
	 * We might need to do something special for the
	 * subtle case of RTS/CTS. I don't know what.
	 *
	 * See section 9.9.2.2.1, 802.11e/D12.1
	 */
	assert (m_last_nav_start_us < now_time_us);
	uint64_t old_nav_start_us = m_last_nav_start_us;
	uint64_t old_nav_end_us = old_nav_start_us + m_last_nav_duration_us;
	uint64_t new_nav_start_us = now_time_us;
	uint64_t duration_us = hdr->get_duration ();

	if (hdr->is_cfpoll () &&
	    hdr->get_addr2 () == m_interface->get_bssid ()) {
		m_last_nav_start_us = new_nav_start_us;
		m_last_nav_duration_us = duration_us;
		for (NavListenersCI i = m_nav_listeners.begin (); i != m_nav_listeners.end (); i++) {
			// XXX !!!!!!!
			(*i)->nav_reset_us (new_nav_start_us, duration_us);
		}
		return;
	}

	if (old_nav_end_us > new_nav_start_us) {
		uint64_t new_nav_end_us = new_nav_start_us + duration_us;
		/* The two NAVs overlap */
		if (new_nav_end_us > old_nav_end_us) {
			for (NavListenersCI i = m_nav_listeners.begin (); i != m_nav_listeners.end (); i++) {
				(*i)->nav_continue_us (new_nav_start_us, duration_us);
			}
		}
	} else {
		m_last_nav_start_us = new_nav_start_us;
		m_last_nav_duration_us = duration_us;
		for (NavListenersCI i = m_nav_listeners.begin (); i != m_nav_listeners.end (); i++) {
			(*i)->nav_start_us (new_nav_start_us, duration_us);
		}
	}
}

void
MacLow::forward_down (Packet const*packet, ChunkMac80211Hdr const* hdr, uint8_t tx_mode, uint8_t stuff)
{
	m_phy->send_packet (packet, tx_mode, 0, stuff);
	/* Note that it is really important to notify the NAV 
	 * thing _after_ forwarding the packet to the PHY.
	 */
	uint64_t tx_duration_us = m_phy->calculate_tx_duration_us (packet->get_size (), tx_mode);
	notify_nav (now_us ()+tx_duration_us, hdr);
}

void
MacLow::cts_timeout (void)
{
	MacStation *station = get_station (m_current_hdr.get_addr1 ());
	station->report_rts_failed ();
	m_current_packet->unref ();
	m_current_packet = 0;
	m_listener->missed_cts ();
	m_listener = 0;
}
void
MacLow::normal_ack_timeout (void)
{
	MacStation *station = get_station (m_current_hdr.get_addr1 ());
	station->report_data_failed ();
	m_listener->missed_ack ();
	m_listener = 0;
}
void
MacLow::fast_ack_timeout (void)
{
	MacStation *station = get_station (m_current_hdr.get_addr1 ());
	station->report_data_failed ();
	if (m_phy->is_state_idle ()) {
		TRACE ("fast Ack idle missed");
		m_listener->missed_ack ();
	}
	m_listener = 0;
}
void
MacLow::super_fast_ack_timeout ()
{
	MacStation *station = get_station (m_current_hdr.get_addr1 ());
	station->report_data_failed ();
	if (m_phy->is_state_idle ()) {
		TRACE ("super fast Ack failed");
		m_listener->missed_ack ();
	} else {
		TRACE ("super fast Ack ok");
		m_listener->got_ack (0.0, 0);
	}
	m_listener = 0;
}

void
MacLow::send_rts_for_packet (void)
{
	/* send an RTS for this packet. */
	ChunkMac80211Hdr rts;
	rts.set_type (MAC_80211_CTL_RTS);
	rts.set_ds_not_from ();
	rts.set_ds_not_to ();
	rts.set_addr1 (m_current_hdr.get_addr1 ());
	rts.set_addr2 (m_interface->get_mac_address ());
	uint8_t rts_tx_mode = get_rts_tx_mode (m_current_hdr.get_addr1 ());
	uint64_t duration_us;
	if (m_tx_params.has_duration_id ()) {
		duration_us = m_tx_params.get_duration_id ();
	} else {
		uint8_t data_tx_mode = get_data_tx_mode (m_current_hdr.get_addr1 (), get_current_size ());
		uint8_t ack_tx_mode = get_ack_tx_mode_for_data (m_current_hdr.get_addr1 (), data_tx_mode);
		uint8_t cts_tx_mode = get_cts_tx_mode_for_rts (m_current_hdr.get_addr1 (), rts_tx_mode);
		duration_us = 0;
		duration_us += get_sifs_us ();
		duration_us += m_phy->calculate_tx_duration_us (get_cts_size (), cts_tx_mode);
		duration_us += get_sifs_us ();
		duration_us += m_phy->calculate_tx_duration_us (get_current_size (), data_tx_mode);
		duration_us += get_sifs_us ();
		duration_us += m_phy->calculate_tx_duration_us (get_ack_size (), ack_tx_mode);
	}
	rts.set_duration_us (duration_us);

	TRACE ("tx RTS to="<< rts.get_addr1 () << ", mode=" << (uint32_t)rts_tx_mode);

	uint64_t tx_duration_us = m_phy->calculate_tx_duration_us (get_rts_size (), rts_tx_mode);
	uint64_t timer_delay_us = tx_duration_us + get_cts_timeout_us ();

	assert (!m_cts_timeout_event.is_running ());
	m_cts_timeout_event = make_event (&MacLow::cts_timeout, this);
	Simulator::insert_in_s (timer_delay_us, m_cts_timeout_event);

	Packet *packet = PacketFactory::create ();
	packet->add (&rts);
	ChunkMac80211Fcs fcs;
	packet->add (&fcs);

	forward_down (packet, &rts, rts_tx_mode, 0);
	packet->unref ();
}

void
MacLow::start_data_tx_timers (void)
{
	uint8_t data_tx_mode = get_data_tx_mode (m_current_hdr.get_addr1 (), get_current_size ());
	uint64_t tx_duration_us = m_phy->calculate_tx_duration_us (get_current_size (), data_tx_mode);
	if (m_tx_params.must_wait_normal_ack ()) {
		uint64_t timer_delay_us = tx_duration_us + get_ack_timeout_us ();
		assert (!m_normal_ack_timeout_event.is_running ());
		m_normal_ack_timeout_event = make_event (&MacLow::normal_ack_timeout, this);
		Simulator::insert_in_us (timer_delay_us, m_normal_ack_timeout_event);
	} else if (m_tx_params.must_wait_fast_ack ()) {
		uint64_t timer_delay_us = tx_duration_us + get_pifs_us ();
		assert (!m_fast_ack_timeout_event.is_running ());
		m_fast_ack_timeout_event = make_event (&MacLow::fast_ack_timeout, this);
		Simulator::insert_in_s (timer_delay_us, m_fast_ack_timeout_event);
	} else if (m_tx_params.must_wait_super_fast_ack ()) {
		uint64_t timer_delay_us = tx_duration_us + get_pifs_us ();
		assert (!m_super_fast_ack_timeout_event.is_running ());
		m_super_fast_ack_timeout_event = make_event (&MacLow::super_fast_ack_timeout, this);
		Simulator::insert_in_s (timer_delay_us, m_super_fast_ack_timeout_event);
	} else if (m_tx_params.has_next_packet ()) {
		uint64_t delay_us = tx_duration_us + get_sifs_us ();
		assert (!m_wait_sifs_event.is_running ());
		m_wait_sifs_event = make_event (&MacLow::wait_sifs_after_end_tx, this);
		Simulator::insert_in_us (delay_us, m_wait_sifs_event);
	} else {
		// since we do not expect any timer to be triggered.
		m_listener = 0;
	}
}

void
MacLow::send_data_packet (void)
{
	/* send this packet directly. No RTS is needed. */
	start_data_tx_timers ();

	uint8_t data_tx_mode = get_data_tx_mode (m_current_hdr.get_addr1 (), get_current_size ());
	TRACE ("tx "<< m_current_hdr.get_type_string () << 
	       ", to=" << m_current_hdr.get_addr1 () <<
	       ", mode=" << (uint32_t)data_tx_mode);
	uint64_t duration_us;
	if (m_tx_params.has_duration_id ()) {
		duration_us = m_tx_params.get_duration_id ();
	} else {
		uint8_t ack_tx_mode = get_ack_tx_mode_for_data (m_current_hdr.get_addr1 (), 
								data_tx_mode);
		duration_us = 0;
		if (m_tx_params.must_wait_ack ()) {
			duration_us += get_sifs_us ();
			duration_us += m_phy->calculate_tx_duration_us (get_ack_size (), ack_tx_mode);
		}
		if (m_tx_params.has_next_packet ()) {
			duration_us += get_sifs_us ();
			duration_us += m_phy->calculate_tx_duration_us (m_tx_params.get_next_packet_size (), 
									data_tx_mode);
			if (m_tx_params.must_wait_ack ()) {
				duration_us += get_sifs_us ();
				duration_us += m_phy->calculate_tx_duration_us (get_ack_size (), ack_tx_mode);
			}
		}
	}
	m_current_hdr.set_duration_us (duration_us);

	m_current_packet->add (&m_current_hdr);
	ChunkMac80211Fcs fcs;
	m_current_packet->add (&fcs);

	forward_down (m_current_packet, &m_current_hdr, data_tx_mode, 0);
	m_current_packet->unref ();
	m_current_packet = 0;
}

bool 
MacLow::is_nav_zero (uint64_t now_time_us)
{
	if (m_last_nav_start_us + m_last_nav_duration_us > now_time_us) {
		return false;
	} else {
		return true;
	}
}

MacStation *
MacLow::get_station (MacAddress ad) const
{
	return m_stations->lookup (ad);
}

void
MacLow::send_cts_after_rts (MacAddress source, uint64_t duration_us, uint8_t tx_mode, uint8_t stuff)
{
	/* send a CTS when you receive a RTS 
	 * right after SIFS.
	 */
	TRACE ("tx CTS to=" << source << ", mode=" << (uint32_t)tx_mode);
	ChunkMac80211Hdr cts;
	cts.set_type (MAC_80211_CTL_CTS);
	cts.set_ds_not_from ();
	cts.set_ds_not_to ();
	cts.set_addr1 (source);
	duration_us -= m_phy->calculate_tx_duration_us (get_cts_size (), tx_mode);
	duration_us -= get_sifs_us ();
	cts.set_duration_us (duration_us);

	Packet *packet = PacketFactory::create ();
	packet->add (&cts);
	ChunkMac80211Fcs fcs;
	packet->add (&fcs);

	forward_down (packet, &cts, tx_mode, stuff);
	packet->unref ();
}

void
MacLow::send_data_after_cts (MacAddress source, uint64_t duration_us, uint8_t tx_mode)
{
	/* send the third step in a 
	 * RTS/CTS/DATA/ACK hanshake 
	 */
	assert (m_current_packet != 0);
	uint8_t data_tx_mode = get_data_tx_mode (m_current_hdr.get_addr1 (), get_current_size ());

	TRACE ("tx " << m_current_hdr.get_type_string () << " to=" << m_current_hdr.get_addr2 () <<
	       ", mode=" << (uint32_t)data_tx_mode << ", seq=0x"<< m_current_hdr.get_sequence_control ());

	start_data_tx_timers ();
	uint64_t tx_duration_us = m_phy->calculate_tx_duration_us (get_current_size (), data_tx_mode);
	duration_us -= tx_duration_us;
	duration_us -= get_sifs_us ();
	m_current_hdr.set_duration_us (duration_us);

	m_current_packet->add (&m_current_hdr);
	ChunkMac80211Fcs fcs;
	m_current_packet->add (&fcs);

	forward_down (m_current_packet, &m_current_hdr, data_tx_mode, 0);
	m_current_packet->unref ();
	m_current_packet = 0;
}

void 
MacLow::wait_sifs_after_end_tx (void)
{
	m_listener->start_next ();
}

void
MacLow::fast_ack_failed_timeout (void)
{
	m_listener->missed_ack ();
	TRACE ("fast Ack busy but missed");
}

void
MacLow::send_ack_after_data (MacAddress source, uint64_t duration_us, uint8_t tx_mode, uint8_t stuff)
{
	/* send an ACK when you receive 
	 * a packet after SIFS. 
	 */
	TRACE ("tx ACK to=" << source << ", mode=" << (uint32_t)tx_mode);
	ChunkMac80211Hdr ack;
	ack.set_type (MAC_80211_CTL_ACK);
	ack.set_ds_not_from ();
	ack.set_ds_not_to ();
	ack.set_addr1 (source);
	duration_us -= m_phy->calculate_tx_duration_us (get_ack_size (), tx_mode);
	duration_us -= get_sifs_us ();
	ack.set_duration_us (duration_us);

	Packet *packet = PacketFactory::create ();
	packet->add (&ack);
	ChunkMac80211Fcs fcs;
	packet->add (&fcs);

	forward_down (packet, &ack, tx_mode, stuff);
	packet->unref ();
}



}; // namespace yans
