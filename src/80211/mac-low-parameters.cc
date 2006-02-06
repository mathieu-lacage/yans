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

#include "mac-low-parameters.h"
#include "phy-80211.h"
#include "mac-80211.h"

#define nopeMAC_DEBUG 1

#ifdef MAC_DEBUG
# define DEBUG(format, ...) \
	printf ("DEBUG " format "\n", ## __VA_ARGS__);
#else /* MAC_DEBUG */
# define DEBUG(format, ...)
#endif /* MAC_DEBUG */


MacLowParameters::MacLowParameters (Phy80211 *phy)
	: m_phy (phy)
{
	DEBUG ("slot %f", getSlotTime ());
	DEBUG ("SIFS %f", getSIFS ());
	DEBUG ("DIFS %f", getDIFS ());
	DEBUG ("EIFS %f", getEIFS ());
	DEBUG ("ACK timeout %f", getACKTimeoutDuration ());
	DEBUG ("CTS timeout %f", getCTSTimeoutDuration ());
	DEBUG ("CWmin %d", getCWmin ());
	DEBUG ("CWmax %d", getCWmax ());
}

Phy80211 *
MacLowParameters::peekPhy (void)
{
	return m_phy;
}

int
MacLowParameters::getACKSize (void) const
{
	return 2+2+6+4;
}
int
MacLowParameters::getRTSSize (void) const
{
	return 2+2+6+6+4;
}
int
MacLowParameters::getCTSSize (void) const
{
	return 2+2+6+4;
}
int
MacLowParameters::getDataHeaderSize (void)
{
	return 3*2+4*6+4;
}
int 
MacLowParameters::getMgtHeaderSize (void)
{
	return 2+2+6+6+6+2+4;
}
double 
MacLowParameters::getSIFS (void)
{
	/* XXX 802.11a */
	return 16e-6;
}
double 
MacLowParameters::getSlotTime (void)
{
	/* XXX 802.11a */
	return 9e-6;
}
double 
MacLowParameters::getEIFS (void)
{
	/* 802.11 section 9.2.10 */
	// XXX check with regard to 802.11a
	return getSIFS () + 
		peekPhy ()->calculateTxDuration (0, getACKSize ()) +
		getDIFS ();
}
double 
MacLowParameters::getDIFS (void)
{
	/* 802.11 section 9.2.10 */
	return getSIFS () + 2 * getSlotTime ();
}
int
MacLowParameters::getCWmin (void)
{
	/* XXX 802.11a */
	return 15;
}
int
MacLowParameters::getCWmax (void)
{
	/* XXX 802.11a */
	return 1023;
}
int 
MacLowParameters::getMaxSSRC (void)
{
	/* XXX */
	return 7;
}

int 
MacLowParameters::getMaxSLRC (void)
{
	/* XXX */
	return 7;
}
int 
MacLowParameters::getRTSCTSThreshold (void)
{
	/* XXX */
	return 0;
}
double
MacLowParameters::getCTSTimeoutDuration (void)
{
	/* XXX */
	return getSIFS () + peekPhy ()->calculateTxDuration (0, getCTSSize ());;
}
double
MacLowParameters::getACKTimeoutDuration (void)
{
	/* XXX */
	return getSIFS () + peekPhy ()->calculateTxDuration (0, getACKSize ());
}

