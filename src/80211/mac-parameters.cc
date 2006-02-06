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

#include <cassert>

#include "mac-parameters.h"
#include "phy-80211.h"
#include "mac-80211.h"
#include "mac-container.h"

#define PARAM_DEBUG 1

#ifdef PARAM_DEBUG
# define DEBUG(format, ...) \
	printf ("DEBUG %d " format "\n", getSelf (), ## __VA_ARGS__);
#else /* MAC_DEBUG */
# define DEBUG(format, ...)
#endif /* MAC_DEBUG */


MacParameters::MacParameters (MacContainer *container)
	: m_container (container)
{
	DEBUG ("slot %f", getSlotTime ());
	DEBUG ("SIFS %f", getSIFS ());
	DEBUG ("ACK timeout %f", getACKTimeoutDuration ());
	DEBUG ("CTS timeout %f", getCTSTimeoutDuration ());
}

int
MacParameters::getSelf (void)
{
	return m_container->selfAddress ();
}
double 
MacParameters::calculateBaseTxDuration (int size)
{
	return m_container->phy ()->calculateTxDuration (0, size);
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
int
MacParameters::getFragmentationThreshold (void)
{
	/* XXX */
	int fragThreshold = 2000;
	// if this assert is not verified, we cannot ensure
	// that every MSDU will be fragmented in less than 
	// 16 packets.
	assert (getMaxMSDUSize () / 16 < fragThreshold);
	return fragThreshold;
}
double
MacParameters::getCTSTimeoutDuration (void)
{
	/* XXX */
	double ctsTimeout = getSIFS ();
	ctsTimeout += calculateBaseTxDuration (getCTSSize ());
	ctsTimeout += 2 * getMaxPropagationDelay ();
	return ctsTimeout;
}
double
MacParameters::getACKTimeoutDuration (void)
{
	/* XXX */
	double ackTimeout = getSIFS ();
	ackTimeout += calculateBaseTxDuration (getACKSize ());
	ackTimeout += 2 * getMaxPropagationDelay ();
	return ackTimeout;
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


double 
MacParameters::getMSDULifetime (void)
{
	return 10; // seconds.
}

double 
MacParameters::getMaxPropagationDelay (void)
{
	// 1000m is the max propagation distance.
	return 1000 / SPEED_OF_LIGHT;
}

int
MacParameters::getMaxMSDUSize (void)
{
	// section 6.2.1.1.2
	return 2304;
}
