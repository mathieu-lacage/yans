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
#include "network-interface-80211.h"
#include "propagation-model.h"
#include "phy-80211.h"
#include "channel-80211.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "packet.h"
#include "ipv4.h"
#include "trace-container.h"
#include "arp.h"
#include "chunk-mac-llc-snap.h"
#include "mac-low.h"
#include "mac-parameters.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "mac-high-adhoc.h"
#include "dcf.h"
#include "dca-txop.h"
#include "mac-queue-80211e.h"

namespace yans {

NetworkInterface80211::NetworkInterface80211 ()
	: m_name (new std::string ("wlan0")),
	  m_bytes_rx (0)
{}

NetworkInterface80211::~NetworkInterface80211 ()
{
	delete m_name;
	delete m_propagation;
	delete m_phy;
	delete m_stations;
	delete m_low;
	delete m_arp;
	delete m_parameters;
	delete m_tx_middle;
	delete m_rx_middle;
}


void 
NetworkInterface80211::connect_to (Channel80211 *channel)
{
	channel->add (m_propagation);
	m_propagation->set_channel (channel);
}
void 
NetworkInterface80211::register_trace (TraceContainer *container)
{
	container->register_ui_variable ("80211-bytes-rx", &m_bytes_rx);
}

MacAddress 
NetworkInterface80211::get_bssid (void) const
{
	// XXX
	return m_bssid;
}
Ssid 
NetworkInterface80211::get_ssid (void) const
{
	return m_ssid;
}
void 
NetworkInterface80211::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
void 
NetworkInterface80211::set_bssid (MacAddress bssid)
{
	m_bssid = bssid;
}


void 
NetworkInterface80211::set_host (Host *host)
{}
void 
NetworkInterface80211::set_mac_address (MacAddress self)
{
	m_self = self;
}
MacAddress
NetworkInterface80211::get_mac_address (void) const
{
	return m_self;
}
std::string const *
NetworkInterface80211::get_name (void)
{
	return m_name;
}
uint16_t 
NetworkInterface80211::get_mtu (void)
{
	// XXX
	return 2500;
}
void 
NetworkInterface80211::set_up   (void)
{}
void 
NetworkInterface80211::set_down (void)
{}
bool 
NetworkInterface80211::is_down (void)
{
	return false;
}
void 
NetworkInterface80211::set_ipv4_handler (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
NetworkInterface80211::set_ipv4_address (Ipv4Address address)
{
	m_ipv4_address = address;
}
void 
NetworkInterface80211::set_ipv4_mask    (Ipv4Mask mask)
{
	m_ipv4_mask = mask;
}
Ipv4Address 
NetworkInterface80211::get_ipv4_address (void)
{
	return m_ipv4_address;
}
Ipv4Mask
NetworkInterface80211::get_ipv4_mask    (void)
{
	return m_ipv4_mask;
}
Ipv4Address 
NetworkInterface80211::get_ipv4_broadcast (void)
{
	uint32_t mask = m_ipv4_mask.get_host_order ();
	uint32_t address = m_ipv4_address.get_host_order ();
	Ipv4Address broadcast = Ipv4Address (address | (~mask));
	return broadcast;
}

void 
NetworkInterface80211::send (Packet *packet, Ipv4Address dest)
{
	m_arp->send_data (packet, dest);
}

void 
NetworkInterface80211::forward_up (Packet *packet)
{
	ChunkMacLlcSnap llc;
	packet->remove (&llc);
	switch (llc.get_ether_type ()) {
	case ETHER_TYPE_ARP:
		m_arp->recv_arp (packet);
		break;
	case ETHER_TYPE_IPV4:
		m_bytes_rx += packet->get_size ();
		m_ipv4->receive (packet, this);
		break;
	}
}
void 
NetworkInterface80211::send_arp (Packet *packet, MacAddress to)
{
	ChunkMacLlcSnap llc;
	llc.set_ether_type (ETHER_TYPE_ARP);
	packet->add (&llc);
	forward_down (packet, to);
}
void 
NetworkInterface80211::send_data (Packet *packet, MacAddress to)
{
	ChunkMacLlcSnap llc;
	llc.set_ether_type (ETHER_TYPE_IPV4);
	packet->add (&llc);
	forward_down (packet, to);
}


NetworkInterface80211Adhoc::NetworkInterface80211Adhoc ()
{}
NetworkInterface80211Adhoc::~NetworkInterface80211Adhoc ()
{
	delete m_dca;
	delete m_high;
}

void 
NetworkInterface80211Adhoc::forward_down (Packet *packet, MacAddress to)
{
	m_high->enqueue (packet, to);
}



}; // namespace yans

