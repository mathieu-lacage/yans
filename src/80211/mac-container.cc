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

#include "mac-container.h"
#include "mac-80211.h"
#include "gpacket.h"


MacContainer::MacContainer (Mac80211 *nsMac)
	: m_nsMac (nsMac)
{}

MacParameters *
MacContainer::parameters (void)
{
	return m_parameters;
}
MacLow *
MacContainer::macLow (void) 
{
	return m_low;
}
MacRxMiddle *
MacContainer::macRxMiddle (void)
{
	return m_middle;
}
MacTxMiddle *
MacContainer::macTxMiddle (void)
{
	return m_txMiddle;
}
MacHigh *
MacContainer::macHigh (void)
{
	return m_high;
}
Phy80211 *
MacContainer::phy (void)
{
	return m_phy;
}
MacStations *
MacContainer::stations (void)
{
	return m_stations;
}

int
MacContainer::getBSSID (void)
{
	return m_bssid;
}
int 
MacContainer::selfAddress (void)
{
	return m_nsMac->addr ();
}
void 
MacContainer::forwardToPhy (Packet *packet)
{
	HDR_CMN (packet)->direction () = hdr_cmn::DOWN;
	m_nsMac->downtarget ()->recv (packet, (Handler *)0);
}
void 
MacContainer::forwardToLL (Packet *packet)
{
	m_nsMac->uptarget ()->recv (packet);
}

	/* this is the order in which these methods
	 * must be invoked for proper intialization.
	 */
void 
MacContainer::setPhy (Phy80211 *phy)
{
	m_phy = phy;
}
void 
MacContainer::setStations (MacStations *stations)
{
	m_stations = stations;
}
void 
MacContainer::setParameters (MacParameters *parameters)
{
	m_parameters = parameters;
}
void 
MacContainer::setMacLow (MacLow *low)
{
	m_low = low;
}
void 
MacContainer::setMacRxMiddle (MacRxMiddle *middle)
{
	m_middle = middle;
}
void 
MacContainer::setMacTxMiddle (MacTxMiddle *middle)
{
	m_txMiddle = middle;
}
void 
MacContainer::setMacHigh (MacHigh *high)
{
	m_high = high;
}
void
MacContainer::setBSSID (int bssid)
{
	m_bssid = bssid;
}
