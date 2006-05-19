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
#include "arp.h"
#include "chunk-mac-llc-snap.h"

namespace yans {

NetworkInterface80211Simple::NetworkInterface80211Simple (MacAddress address)
	: MacNetworkInterface (address, 1000),
	  m_bytes_rx (0)
{}

NetworkInterface80211Simple::~NetworkInterface80211Simple ()
{
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
NetworkInterface80211Simple::notify_up (void)
{}
void 
NetworkInterface80211Simple::notify_down (void)
{}
void 
NetworkInterface80211Simple::real_send (Packet *packet, MacAddress to)
{
	m_mac->send (packet, to);
}

void 
NetworkInterface80211Simple::forward_data_up (Packet *packet)
{
	m_bytes_rx += packet->get_size ();
	MacNetworkInterface::forward_up (packet);
}

}; // namespace yans

