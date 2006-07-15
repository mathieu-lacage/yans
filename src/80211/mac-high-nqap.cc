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
#include "mac-high-nqap.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "dca-txop.h"
#include "network-interface-80211.h"
#include "chunk-mac-80211-hdr.h"
#include "chunk-mgt.h"
#include <cassert>

#define noNQAP_DEBUG 1

#ifdef NQAP_DEBUG
#include <iostream>
#include "simulator.h"
#  define TRACE(x) \
std::cout << "NQAP now=" << Simulator::now_us () << "us " << x << std::endl;
#else
#  define TRACE(x)
#endif


namespace yans {

MacHighNqap::MacHighNqap ()
	: m_beacon_interval_us (500000)
{}
MacHighNqap::~MacHighNqap ()
{}

void 
MacHighNqap::set_dca_txop (DcaTxop *dca)
{
	m_dca = dca;
	m_dca->set_tx_ok_callback (make_callback (&MacHighNqap::tx_ok, this));
	m_dca->set_tx_failed_callback (make_callback (&MacHighNqap::tx_failed, this));
}
void 
MacHighNqap::set_interface (NetworkInterface80211 *interface)
{
	m_interface = interface;
}
void 
MacHighNqap::set_stations (MacStations *stations)
{
	m_stations = stations;
}
void 
MacHighNqap::set_forward_callback (ForwardCallback callback)
{
	m_forward_up = callback;
}
void 
MacHighNqap::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}
void 
MacHighNqap::set_beacon_interval_us (uint64_t us)
{
	m_beacon_interval_us = us;
}
void 
MacHighNqap::forward_down (PacketPtr packet, MacAddress from, MacAddress to)
{
	ChunkMac80211Hdr hdr;
	hdr.set_type_data ();
	hdr.set_addr1 (to);
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (from);
	hdr.set_ds_from ();
	hdr.set_ds_not_to ();
	m_dca->queue (packet, hdr);	
}
void 
MacHighNqap::queue (PacketPtr packet, MacAddress to)
{
	forward_down (packet, m_interface->get_mac_address (), to);
}
SupportedRates
MacHighNqap::get_supported_rates (void)
{
	return m_rates;
}
void
MacHighNqap::send_probe_resp (MacAddress to)
{
	TRACE ("send probe response to="<<to);
	ChunkMac80211Hdr hdr;
	hdr.set_probe_resp ();
	hdr.set_addr1 (to);
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (m_interface->get_mac_address ());
	hdr.set_ds_not_from ();
	hdr.set_ds_not_to ();
	PacketPtr packet = Packet::create ();
	ChunkMgtProbeResponse probe;
	probe.set_ssid (m_interface->get_ssid ());
	SupportedRates rates = get_supported_rates ();
	probe.set_supported_rates (rates);
	probe.set_beacon_interval_us (m_beacon_interval_us);
	packet->add (&probe);
	
	m_dca->queue (packet, hdr);
}
void
MacHighNqap::send_assoc_resp (MacAddress to)
{
	TRACE ("send assoc response to="<<to);
	ChunkMac80211Hdr hdr;
	hdr.set_assoc_resp ();
	hdr.set_addr1 (to);
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (m_interface->get_mac_address ());
	hdr.set_ds_not_from ();
	hdr.set_ds_not_to ();
	PacketPtr packet = Packet::create ();
	ChunkMgtAssocResponse assoc;
	StatusCode code;
	code.set_success ();
	assoc.set_status_code (code);
	packet->add (&assoc);
	
	m_dca->queue (packet, hdr);
}
void 
MacHighNqap::tx_ok (ChunkMac80211Hdr const &hdr)
{
	MacStation *station = m_stations->lookup (hdr.get_addr1 ());
	if (hdr.is_assoc_resp () && 
	    station->is_wait_assoc_tx_ok ()) {
		TRACE ("associated with sta="<<hdr.get_addr1 ());
		station->record_got_assoc_tx_ok ();
	}
}
void 
MacHighNqap::tx_failed (ChunkMac80211Hdr const &hdr)
{
	MacStation *station = m_stations->lookup (hdr.get_addr1 ());
	if (hdr.is_assoc_resp () && 
	    station->is_wait_assoc_tx_ok ()) {
		TRACE ("assoc failed with sta="<<hdr.get_addr1 ());
		station->record_got_assoc_tx_failed ();
	}
}
void 
MacHighNqap::receive (PacketPtr packet, ChunkMac80211Hdr const *hdr)
{
	MacStation *station = m_stations->lookup (hdr->get_addr2 ());

	if (hdr->is_data ()) {
		if (!hdr->is_from_ds () && 
		    hdr->is_to_ds () &&
		    hdr->get_addr1 () == m_interface->get_mac_address () &&
		    station->is_associated ()) {
			if (hdr->get_addr3 () == m_interface->get_mac_address ()) {
				m_forward_up (packet);
			} else {
				forward_down (packet->copy (), 
					      hdr->get_addr2 (), 
					      hdr->get_addr3 ());
				m_forward_up (packet);
			}
		} else if (hdr->is_from_ds () &&
			   hdr->is_to_ds ()) {
			// this is an AP-to-AP frame
			// we ignore for now.
		} else {
			// we can ignore these frames since 
			// they are not targeted at the AP
		}
	} else if (hdr->is_mgt ()) {
		if (hdr->is_probe_req ()) {
			assert (hdr->get_addr1 ().is_broadcast ());
			send_probe_resp (hdr->get_addr2 ());
		} else if (hdr->get_addr1 () == m_interface->get_mac_address ()) {
			if (hdr->is_assoc_req ()) {
				station->record_wait_assoc_tx_ok ();
				send_assoc_resp (hdr->get_addr2 ());
			} else if (hdr->is_disassociation ()) {
				station->record_disassociated ();
			} else if (hdr->is_reassoc_req ()) {
				/* we don't support reassoc frames for now */
			} else if (hdr->is_authentication () ||
				   hdr->is_deauthentication ()) {
				/*
				 */
			} else {
				/* unknown mgt frame
				 */
			}
		}
	} else {
		/* damn, what could this be ? a control frame ?
		 * control frames should never reach the MacHigh so,
		 * this is likely to be a bug. Assert.
		 */
		assert (false);
	}	
}



}; // namespace yans
