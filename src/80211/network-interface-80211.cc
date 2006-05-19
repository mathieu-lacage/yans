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
#include "trace-container.h"
#include "mac-low.h"
#include "mac-parameters.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "mac-high-adhoc.h"
#include "mac-high-nqsta.h"
#include "mac-high-nqap.h"
#include "dca-txop.h"

namespace yans {

NetworkInterface80211::NetworkInterface80211 (MacAddress address)
	: MacNetworkInterface (address, 1000),
	  m_bytes_rx (0)
{}

NetworkInterface80211::~NetworkInterface80211 ()
{
	delete m_propagation;
	delete m_phy;
	delete m_stations;
	delete m_low;
	delete m_parameters;
	delete m_tx_middle;
	delete m_rx_middle;
}

void 
NetworkInterface80211::notify_up (void)
{}
void 
NetworkInterface80211::notify_down (void)
{}

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
void 
NetworkInterface80211::forward_up_data (Packet *packet)
{
	m_bytes_rx += packet->get_size ();
	MacNetworkInterface::forward_up (packet);
}


NetworkInterface80211Adhoc::NetworkInterface80211Adhoc (MacAddress address)
	: NetworkInterface80211 (address)
{}
NetworkInterface80211Adhoc::~NetworkInterface80211Adhoc ()
{
	delete m_dca;
	delete m_high;
}
MacAddress 
NetworkInterface80211Adhoc::get_bssid (void) const
{
	return m_high->get_bssid ();
}
Ssid 
NetworkInterface80211Adhoc::get_ssid (void) const
{
	return m_ssid;
}
void 
NetworkInterface80211Adhoc::set_ssid (Ssid ssid)
{
	// XXX restart adhoc network join.
	m_ssid = ssid;
}
void 
NetworkInterface80211Adhoc::real_send (Packet *packet, MacAddress to)
{
	m_high->enqueue (packet, to);
}



NetworkInterface80211Nqsta::NetworkInterface80211Nqsta (MacAddress address)
	: NetworkInterface80211 (address)
{}
NetworkInterface80211Nqsta::~NetworkInterface80211Nqsta ()
{
	delete m_dca;
	delete m_high;
}
MacAddress 
NetworkInterface80211Nqsta::get_bssid (void) const
{
	return m_high->get_bssid ();
}
Ssid 
NetworkInterface80211Nqsta::get_ssid (void) const
{
	return m_ssid;
}
void 
NetworkInterface80211Nqsta::set_ssid (Ssid ssid)
{
	// XXX restart ap scan
	m_ssid = ssid;
}
void 
NetworkInterface80211Nqsta::real_send (Packet *packet, MacAddress to)
{
	m_high->queue (packet, to);
}
void 
NetworkInterface80211Nqsta::associated (void)
{
	//XXX should flush the arp cache here.
}


NetworkInterface80211Nqap::NetworkInterface80211Nqap (MacAddress address)
	: NetworkInterface80211 (address)
{}
NetworkInterface80211Nqap::~NetworkInterface80211Nqap ()
{
	delete m_dca;
	delete m_high;
}
MacAddress 
NetworkInterface80211Nqap::get_bssid (void) const
{
	return get_mac_address ();
}
Ssid 
NetworkInterface80211Nqap::get_ssid (void) const
{
	return m_ssid;
}
void 
NetworkInterface80211Nqap::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
void 
NetworkInterface80211Nqap::real_send (Packet *packet, MacAddress to)
{
	m_high->queue (packet, to);
}


}; // namespace yans

