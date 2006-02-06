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
MacParameters::getPacketSize (enum mac_80211_packet_type type)
{
	assert (isManagement (type) || isControl (type));
	int size;
	switch (type) {
	case MAC_80211_CTL_RTS:
		size = getRTSSize ();
		break;
	case MAC_80211_CTL_CTS:
		size = getCTSSize ();
		break;
	case MAC_80211_CTL_ACK:
		size = getACKSize ();
		break;
	case MAC_80211_CTL_BACKREQ:
	case MAC_80211_CTL_BACKRESP:
		/* not implemented */
		size = 0;
		assert (false);
		break;

	case MAC_80211_DATA:
		/* cannot happen since assert at start of method */
		size = 0;
		assert (false);
		break;

	case MAC_80211_MGT_ADDBA_REQUEST:
	case MAC_80211_MGT_ADDBA_RESPONSE:
	case MAC_80211_MGT_DELBA_REQUEST:
	case MAC_80211_MGT_DELBA_RESPONSE:
	case MAC_80211_MGT_AUTHENTICATION:
	case MAC_80211_MGT_DEAUTHENTICATION:
	case MAC_80211_MGT_CFPOLL:
	case MAC_80211_MGT_BEACON:
	case MAC_80211_MGT_ASSOCIATION_REQUEST:
	case MAC_80211_MGT_ASSOCIATION_RESPONSE:
	case MAC_80211_MGT_DISASSOCIATION:
	case MAC_80211_MGT_REASSOCIATION_REQUEST:
	case MAC_80211_MGT_REASSOCIATION_RESPONSE:
	case MAC_80211_MGT_PROBE_REQUEST:
	case MAC_80211_MGT_PROBE_RESPONSE:
	case MAC_80211_MGT_ADDTS_REQUEST:
	case MAC_80211_MGT_ADDTS_RESPONSE:
	case MAC_80211_MGT_DELTS_REQUEST:
	case MAC_80211_MGT_DELTS_RESPONSE:
		size = getMgtHeaderSize ();
		size += getManagementPayloadSize (type);
		break;
	default:
		assert (false);
		break;
	}

	assert (size < getMaxMSDUSize ());

	return size;
}
int 
MacParameters::getManagementPayloadSize (enum mac_80211_packet_type type)
{
	assert (isManagement (type));
	int size;
	int tspecSize = 0 +
		1 + // element id
		1 + // length
		3 + // ts info
		2 + // nominal msdu size
		2 + // maximum msdu size
		4 + // minimum service interval
		4 + // maximum service interval
		4 + // inactivity interval
		4 + // suspension interval
		4 + // service start time
		4 + // minimum data rate
		4 + // mean data rate
		4 + // peak data rate
		4 + // maximum burst size
		4 + // delay bound
		4 + // minimum phy rate
		2 + // surplus bandwidth allowance
		2 + // medium time
		0;
	switch (type) {
	default:
	case MAC_80211_MGT_ADDBA_REQUEST:
	case MAC_80211_MGT_ADDBA_RESPONSE:
	case MAC_80211_MGT_DELBA_REQUEST:
	case MAC_80211_MGT_DELBA_RESPONSE:
	case MAC_80211_MGT_AUTHENTICATION:
	case MAC_80211_MGT_DEAUTHENTICATION:
	case MAC_80211_MGT_DISASSOCIATION:
		// XXX not implemented
		size = 0;
		assert (false);
		break;
	case MAC_80211_MGT_CFPOLL:
		// QOS CFPOLL has no data.
		size = 0;
		break;
	case MAC_80211_MGT_QOSNULL:
		size = 0;
		break;
	case MAC_80211_MGT_BEACON:
		size = getBeaconSize ();
		break;
	case MAC_80211_MGT_ASSOCIATION_REQUEST:
		size = getAssociationRequestSize ();
		break;
	case MAC_80211_MGT_ASSOCIATION_RESPONSE:
		size = getAssociationResponseSize ();
		break;
	case MAC_80211_MGT_REASSOCIATION_REQUEST:
		size = getReAssociationRequestSize ();
		break;
	case MAC_80211_MGT_REASSOCIATION_RESPONSE:
		size = getReAssociationResponseSize ();
		break;
	case MAC_80211_MGT_PROBE_REQUEST:
		size = getProbeRequestSize ();
		break;
	case MAC_80211_MGT_PROBE_RESPONSE:
		size = getProbeResponseSize ();
		break;
	case MAC_80211_MGT_ADDTS_REQUEST:
		size =  1 + // category
			1 + // action
			1 + // dialog token
			tspecSize + 
			// no TCLAS
			0;
		break;
	case MAC_80211_MGT_ADDTS_RESPONSE:
		size =  1 + // category
			1 + // action
			1 + // dialog token
			2 + // status code
			1 + 1 + 4 + // ts delay
			tspecSize + 
			// no TCLAS and no schedule
			0;
		break;
	case MAC_80211_MGT_DELTS_REQUEST:
		size =  1 + // category
			1 + // action
			3 + // tsinfo
			2 + // reason code
			0;
		break;
	case MAC_80211_MGT_DELTS_RESPONSE:
		size =  1 + // category
			1 + // action
			3 + // tsinfo
			2 + // reason code
			0;
		break;
	}

	return size;
}
int
MacParameters::getDataHeaderSize (void)
{
	return 3*2+4*6+4;
}
double
MacParameters::getPIFS (void)
{
	/* see section 9.2.10 ieee 802.11-1999 */
	return getSIFS () + getSlotTime ();
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
	return 1000;
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


double 
MacParameters::getCapLimit (void)
{
	return 0.4;
}

double 
MacParameters::getMinEdcaTrafficProportion (void)
{
	return 0.4;
}



/* deprecated. Use the getSize (type) method instead. */
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
int 
MacParameters::getMgtHeaderSize (void)
{
	return 2+2+6+6+6+2+4;
}
