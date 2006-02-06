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

#include "mac-parameters.h"
#include "phy-80211.h"

MacParameters::MacParameters (Phy80211 *phy)
	: m_phy (phy)
{}

Phy80211 *
MacParameters::peekPhy (void)
{
	return m_phy;
}

int
MacParameters::getACKSize (void) const
{
	return 2+2+6+4;
}
int
MacParameters::getRTSSize (void) const
{
	return 2+2+6+6+4;
}
int
MacParameters::getCTSSize (void) const
{
	return 2+2+6+4;
}
int
MacParameters::getDataHeaderSize (void)
{
	return 3*2+4*6+4;
}
int 
MacParameters::getMgtHeaderSize (void)
{
	return 2+2+6+6+6+2+4;
}
double 
MacParameters::getSIFS (void)
{
	/* XXX 802.11a */
	return 16e-6;
}
double 
MacParameters::getSlotTime (void)
{
	/* XXX 802.11a */
	return 9e-6;
}
double 
MacParameters::getEIFS (void)
{
	/* 802.11 section 9.2.10 */
	// XXX check with regard to 802.11a
	return getSIFS () + 
		peekPhy ()->calculateTxDuration (0, getACKSize ()) +
		getDIFS ();
}
double 
MacParameters::getDIFS (void)
{
	/* 802.11 section 9.2.10 */
	return getSIFS () + 2 * getSlotTime ();
}
int
MacParameters::getCWmin (void)
{
	/* XXX 802.11a */
	return 15;
}
int
MacParameters::getCWmax (void)
{
	/* XXX 802.11a */
	return 1023;
}
int 
MacParameters::getMaxSSRC (void)
{
	/* XXX */
	return 7;
}

int 
MacParameters::getMaxSLRC (void)
{
	/* XXX */
	return 7;
}
int 
MacParameters::getRTSCTSThreshold (void)
{
	/* XXX */
	return 0;
}
double
MacParameters::getCTSTimeoutDuration (void)
{
	/* XXX */
	return getSIFS () + peekPhy ()->calculateTxDuration (0, getCTSSize ());;
}
double
MacParameters::getACKTimeoutDuration (void)
{
	/* XXX */
	return getSIFS () + peekPhy ()->calculateTxDuration (0, getACKSize ());
}


double
MacParameters::getBeaconInterval (void)
{
	// XXX
	return 1.0;
}
int
MacParameters::getMaxMissedBeacon (void)
{
	// XXX
	return 4;
}


int
MacParameters::getBeaconSize (void)
{
	//XXX
	return getProbeResponseSize ()+
		256+// max DTIM
		0;
}
int
MacParameters::getAssociationResponseSize (void)
{
	//XXX
	return 2+//capability
		+//status
		2+//associationID
		10+//max supported rates
		0;
}
int
MacParameters::getReAssociationResponseSize (void)
{
	//XXX
	return getAssociationResponseSize ();
}
int
MacParameters::getProbeResponseSize (void)
{
	//XXX
	return 8+//timestamp
		2+//interval
		2+//capability
		34+// max SSID
		10+// max supported rates
		0+//phy parameter set
		0;
}
int
MacParameters::getAssociationRequestSize (void)
{
	//XXX
	return 2+//capability
		2+//listen interval
		34+//max ssid
		10+//max supported rates
		0;
}
int
MacParameters::getReAssociationRequestSize (void)
{
	//XXX
	return getAssociationRequestSize ()+
		6+//ap address
		0;
}
int
MacParameters::getProbeRequestSize (void)
{
	//XXX
	return 34+//max ssid
		10+//supported rates
		0;
}

