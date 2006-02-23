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
#include "network-interface-80211-simple.h"
#include "propagation-model.h"
#include "phy-80211.h"
#include "channel-80211.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "packet.h"
#include "ipv4.h"
#include "trace-container.h"
#include "mac-simple.h"

namespace yans {

NetworkInterface80211Simple::NetworkInterface80211Simple ()
	: m_name (new std::string ("wlan0")),
	  m_bytes_rx (0)
{}

NetworkInterface80211Simple::~NetworkInterface80211Simple ()
{
	delete m_name;
	delete m_propagation;
	delete m_phy;
	delete m_stations;
	delete m_mac;
}


void 
NetworkInterface80211Simple::connect_to (Channel80211 *channel)
{
	channel->add (m_propagation);
	m_propagation->set_channel (channel);
}
void 
NetworkInterface80211Simple::register_trace (TraceContainer *container)
{
	container->register_ui_variable ("80211-bytes-rx", &m_bytes_rx);
}

void 
NetworkInterface80211Simple::set_host (Host *host)
{}
void 
NetworkInterface80211Simple::set_mac_address (MacAddress self)
{
	m_self = self;
}
MacAddress 
NetworkInterface80211Simple::get_mac_address (void)
{
	return m_self;
}
std::string const *
NetworkInterface80211Simple::get_name (void)
{
	return m_name;
}
uint16_t 
NetworkInterface80211Simple::get_mtu (void)
{
	// XXX
	return 2500;
}
void 
NetworkInterface80211Simple::set_up   (void)
{}
void 
NetworkInterface80211Simple::set_down (void)
{}
bool 
NetworkInterface80211Simple::is_down (void)
{
	return false;
}
void 
NetworkInterface80211Simple::set_ipv4_handler (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
NetworkInterface80211Simple::set_ipv4_address (Ipv4Address address)
{
	m_ipv4_address = address;
}
void 
NetworkInterface80211Simple::set_ipv4_mask    (Ipv4Mask mask)
{
	m_ipv4_mask = mask;
}
Ipv4Address 
NetworkInterface80211Simple::get_ipv4_address (void)
{
	return m_ipv4_address;
}
Ipv4Mask
NetworkInterface80211Simple::get_ipv4_mask    (void)
{
	return m_ipv4_mask;
}
Ipv4Address 
NetworkInterface80211Simple::get_ipv4_broadcast (void)
{
	uint32_t mask = m_ipv4_mask.get_host_order ();
	uint32_t address = m_ipv4_address.get_host_order ();
	Ipv4Address broadcast = Ipv4Address (address | (~mask));
	return broadcast;
}

void 
NetworkInterface80211Simple::send (Packet *packet, Ipv4Address dest)
{
	m_mac->send (packet);
}

void 
NetworkInterface80211Simple::rx_phy_ok (Packet *packet, double snr, uint8_t tx_mode)
{
	m_bytes_rx += packet->get_size ();
	m_ipv4->receive (packet, this);
}
void 
NetworkInterface80211Simple::rx_phy_error (Packet *packet)
{}



}; // namespace yans

