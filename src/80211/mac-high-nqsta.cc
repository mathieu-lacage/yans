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
#include "mac-high-nqsta.h"
#include "chunk-mac-80211-hdr.h"
#include "network-interface-80211.h"
#include "packet.h"
#include "chunk-mgt.h"
#include "phy-80211.h"
#include "cancellable-event.h"
#include "cancellable-event.tcc"
#include "dca-txop.h"
#include "simulator.h"
#include "timeout.h"

#include <cassert>

#define noNQSTA_DEBUG 1

#ifdef NQSTA_DEBUG
#include <iostream>
#  define TRACE(x) \
std::cout << "NQSTA " << Simulator::now_us () << "us " << x << std::endl;
#else
#  define TRACE(x)
#endif

/*
 * The state machine for this NQSTA is:
 --------------                                          -----------
 | Associated |   <--------------------      ------->    | Refused |
 --------------                        \    /            -----------
    \                                   \  /
     \    -----------------     -----------------------------
      \-> | Beacon Missed | --> | Wait Association Response |
          -----------------     -----------------------------
	        \                       ^
		 \                      |
		  \    -----------------------
	           \-> | Wait Probe Response |
		       -----------------------
 */


namespace yans {

MacHighNqsta::MacHighNqsta ()
	: m_state (BEACON_MISSED),
	  m_probe_request_timeout_us (500000), // 0.5s
	  m_assoc_request_timeout_us (500000), // 0.5s
	  m_probe_request_event (0),
	  m_assoc_request_event (0),
	  m_timeout (new Timeout (make_callback (&MacHighNqsta::missed_beacons, this)))
{
	// this is the default value for the number of beacons missed 
	// before attempting to reassociate.
	m_timeout->set_count (10);
}

MacHighNqsta::~MacHighNqsta ()
{}

void 
MacHighNqsta::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}
void 
MacHighNqsta::set_dca_txop (DcaTxop *dca)
{
	m_dca = dca;
}
void 
MacHighNqsta::set_interface (NetworkInterface80211 *interface)
{
	m_interface = interface;
}
void 
MacHighNqsta::set_forward_callback (ForwardCallback callback)
{
	m_forward = callback;
}
void 
MacHighNqsta::set_associated_callback (AssociatedCallback callback)
{
	m_associated_callback = callback;
}
void 
MacHighNqsta::set_max_missed_beacons (uint32_t missed)
{
	m_timeout->set_count (missed);
}
void 
MacHighNqsta::set_probe_request_timeout (uint64_t us)
{
	m_probe_request_timeout_us = us;
}
void 
MacHighNqsta::set_assoc_request_timeout (uint64_t us)
{
	m_assoc_request_timeout_us = us;
}

MacAddress 
MacHighNqsta::get_bssid (void) const
{
	return m_bssid;
}
void 
MacHighNqsta::set_bssid (MacAddress bssid)
{
	m_bssid = bssid;
}

MacAddress
MacHighNqsta::get_broadcast_bssid (void)
{
	return MacAddress::get_broadcast ();
}
SupportedRates
MacHighNqsta::get_supported_rates (void)
{
	return m_rates;
}

void
MacHighNqsta::send_probe_request (void)
{
	TRACE ("send probe request");
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	hdr.set_addr1 (get_broadcast_bssid ());
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (get_broadcast_bssid ());
	hdr.set_ds_not_from ();
	hdr.set_ds_to ();
	Packet *packet = new Packet ();
	ChunkMgtProbeRequest probe;
	probe.set_ssid (m_interface->get_ssid ());
	SupportedRates rates = get_supported_rates ();
	probe.set_supported_rates (rates);
	packet->add (&probe);
	
	m_dca->queue (packet, hdr);

	m_probe_request_event = make_cancellable_event (&MacHighNqsta::probe_request_timeout, this);
	Simulator::insert_in_us (m_probe_request_timeout_us,
				 m_probe_request_event);
}

void
MacHighNqsta::send_association_request ()
{
	TRACE ("send assoc request");
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	hdr.set_addr1 (get_bssid ());
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (get_bssid ());
	hdr.set_ds_not_from ();
	hdr.set_ds_to ();
	Packet *packet = new Packet ();
	ChunkMgtAssocRequest assoc;
	assoc.set_ssid (m_interface->get_ssid ());
	SupportedRates rates = get_supported_rates ();
	assoc.set_supported_rates (rates);
	packet->add (&assoc);
	
	m_dca->queue (packet, hdr);

	m_probe_request_event = make_cancellable_event (&MacHighNqsta::probe_request_timeout, this);
	Simulator::insert_in_us (m_probe_request_timeout_us,
				 m_probe_request_event);
}
void
MacHighNqsta::try_to_ensure_associated (void)
{
	switch (m_state) {
	case ASSOCIATED:
		return;
		break;
	case WAIT_PROBE_RESP:
		/* we have sent a probe request earlier so we
		   do not need to re-send a probe request immediately.
		   We just need to wait until probe-request-timeout
		   or until we get a probe response
		 */
		break;
	case BEACON_MISSED:
		/* we were associated but we missed a bunch of beacons
		 * so we should assume we are not associated anymore.
		 * We try to initiate a probe request now.
		 */
		m_state = WAIT_PROBE_RESP;
		send_probe_request ();
		break;
	case WAIT_ASSOC_RESP:
		/* we have sent an assoc request so we do not need to
		   re-send an assoc request right now. We just need to
		   wait until either assoc-request-timeout or until
		   we get an assoc response.
		 */
		break;
	case REFUSED:
		/* we have sent an assoc request and received a negative
		   assoc resp. We wait until someone restarts an 
		   association with a given ssid.
		 */
		break;
	}
}

void
MacHighNqsta::assoc_request_timeout (void)
{
	m_assoc_request_event = 0;
	m_state = WAIT_ASSOC_RESP;
	send_association_request ();
}
void
MacHighNqsta::probe_request_timeout (void)
{
	m_probe_request_event = 0;
	m_state = WAIT_PROBE_RESP;
	send_probe_request ();
}
void 
MacHighNqsta::missed_beacons (void)
{
	m_state = BEACON_MISSED;
}
bool
MacHighNqsta::is_associated (void)
{
	return (m_state == ASSOCIATED)?true:false;
}

void 
MacHighNqsta::queue (Packet *packet, MacAddress to)
{
	if (!is_associated ()) {
		try_to_ensure_associated ();
		return;
	}
	TRACE ("enqueue size="<<packet->get_size ()<<", to="<<to<<
	       ", queue_size="<<m_queue->get_size ());
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	hdr.set_addr1 (get_bssid ());
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (to);
	hdr.set_ds_not_from ();
	hdr.set_ds_to ();
	m_dca->queue (packet, hdr);
}

void 
MacHighNqsta::ack_received (ChunkMac80211Hdr const &hdr)
{}
void 
MacHighNqsta::receive (Packet *packet, ChunkMac80211Hdr const *hdr)
{
	assert (!hdr->is_ctl ());
	if (hdr->get_addr1 () != m_interface->get_mac_address () ||
	    hdr->get_addr1 ().is_broadcast ()) {
		// packet is not for us
	} else if (hdr->is_data ()) {
		m_forward (packet);
	} else if (hdr->is_probe_req () ||
		   hdr->is_assoc_req ()) {
		// we cannot deal with requests.
		assert (false);
	} else if (hdr->is_beacon ()) {
		ChunkMgtBeacon beacon;
		packet->remove (&beacon);
		bool good_beacon = false;
		if (m_interface->get_ssid ().is_broadcast ()) {
			// we do not have any special ssid so this
			// beacon is as good as another.
			m_timeout->restart ();
			m_timeout->set_interval (beacon.get_beacon_interval_us ());
			good_beacon = true;
		} else if (beacon.get_ssid ().is_equal (m_interface->get_ssid ())) {
			//beacon for our ssid.
			m_timeout->restart ();
			m_timeout->set_interval (beacon.get_beacon_interval_us ());
			good_beacon = true;
		}
		if (good_beacon) {
			set_bssid (hdr->get_addr3 ());
		}
		if (good_beacon && m_state == BEACON_MISSED) {
			m_state = WAIT_ASSOC_RESP;
			send_association_request ();
		}
	} else if (hdr->is_probe_resp ()) {
		if (m_state == WAIT_PROBE_RESP) {
			ChunkMgtProbeResponse probe_resp;
			packet->remove (&probe_resp);
			if (!probe_resp.get_ssid ().is_equal (m_interface->get_ssid ())) {
				//not a probe resp for our ssid.
				return;
			}
			set_bssid (hdr->get_addr3 ());
			m_timeout->set_interval (probe_resp.get_beacon_interval_us ());
			if (m_probe_request_event != 0) {
				m_probe_request_event->cancel ();
				m_probe_request_event = 0;
			}
			m_state = WAIT_ASSOC_RESP;
			send_association_request ();
		}
	} else if (hdr->is_assoc_resp ()) {
		if (m_state == WAIT_ASSOC_RESP) {
			ChunkMgtAssocResponse assoc_resp;
			packet->remove (&assoc_resp);
			if (m_assoc_request_event != 0) {
				m_assoc_request_event->cancel ();
				m_assoc_request_event = 0;
			}
			if (assoc_resp.is_success ()) {
				m_state = ASSOCIATED;
				m_associated_callback ();
			} else {
				m_state = REFUSED;
			}
		}
	}
}

}; // namespace yans
