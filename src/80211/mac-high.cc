/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include "mac-80211.h"
#include "mac-high.h"
#include "mac-station.h"
#include "mac-stations.h"
#include "arf-mac-stations.h"
#include "hdr-mac-80211.h"

MacHigh::MacHigh (Mac80211 *mac, Phy80211 *phy)
	: m_mac (mac),
	  m_phy (phy)
{
	m_stations = new ArfMacStations (mac);
}
MacHigh::~MacHigh ()
{}

MacStation *
MacHigh::lookupStation (int address)
{
	return m_stations->lookup (address);
}

Mac80211 *
MacHigh::peekMac80211 (void)
{
	return m_mac;
}

Phy80211 *
MacHigh::peekPhy80211 (void)
{
	return m_phy;
}

int
MacHigh::getSelf (void)
{
	return m_mac->addr ();
}

void 
MacHigh::forwardUp (Packet *packet)
{
	return m_mac->uptarget ()->recv (packet);
}

Packet *
MacHigh::getPacketFor (int destination)
{
	Packet *packet = Packet::alloc ();
	setSource (packet, getSelf ());
	setFinalDestination (packet, destination);
	setDestination (packet, destination);
	return packet;
}

MacParameters *
MacHigh::parameters (void)
{
	return m_mac->parameters ();
}
