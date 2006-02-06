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

#include "mac-high-access-point.h"
#include "mac-80211.h"
#include "arf-mac-stations.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "mac-low.h"
#include "mac-parameters.h"
#include "mac-low-parameters.h"

#include "packet.h"


#define nopeAP_TRACE 1

#ifdef AP_TRACE
# define TRACE(format, ...) \
	printf ("AP TRACE %d " format "\n", getSelf (), ## __VA_ARGS__);
#else /* AP_TRACE */
# define TRACE(format, ...)
#endif /* AP_TRACE */


MacHighAccessPoint::MacHighAccessPoint (Mac80211 *mac, Phy80211 *phy)
	: MacHigh (mac, phy)
{
	m_lowParameters = new MacLowParameters (phy);
	m_low = new MacLow (mac, this, phy, m_lowParameters);
	m_lowBeacon = new MacLow (mac, this, phy, m_lowParameters);
	m_sendBeaconTimer = new StaticHandler<MacHighAccessPoint> (this, &MacHighAccessPoint::sendBeacon);
	m_sendBeaconTimer->start (parameters ()->getBeaconInterval ());
}
MacHighAccessPoint::~MacHighAccessPoint ()
{}

void
MacHighAccessPoint::sendBeacon (void)
{
	Packet *packet = getPacketFor (MAC_BROADCAST);
	setSize (packet, parameters ()->getBeaconSize ());
	setType (packet, MAC_80211_MGT_BEACON);
	TRACE ("queue %s to %d/%d from %d", 
	       getTypeString (packet), 
	       getDestination (packet), 
	       getFinalDestination (packet), 
	       getSource (packet));
	m_lowBeacon->enqueue (packet);

	m_sendBeaconTimer->start (parameters ()->getBeaconInterval ());
}

Packet *
MacHighAccessPoint::getPacketFor (int destination)
{
	Packet *packet = Packet::alloc ();
	setSource (packet, getSelf ());
	setFinalDestination (packet, destination);
	setDestination (packet, destination);
	return packet;
}

void
MacHighAccessPoint::sendAssociationResponseOk (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getAssociationResponseSize ());
	setType (packet, MAC_80211_MGT_ASSOCIATION_RESPONSE);
	queueToLow (packet);
}

void
MacHighAccessPoint::sendReAssociationResponseOk (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getReAssociationResponseSize ());
	setType (packet, MAC_80211_MGT_REASSOCIATION_RESPONSE);
	queueToLow (packet);
}

void
MacHighAccessPoint::sendProbeResponse (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getProbeResponseSize ());
	setType (packet, MAC_80211_MGT_PROBE_RESPONSE);
	queueToLow (packet);
}

void
MacHighAccessPoint::notifyAckReceivedFor (Packet *packet)
{
	if (getType (packet) == MAC_80211_MGT_ASSOCIATION_RESPONSE ||
	    getType (packet) == MAC_80211_MGT_REASSOCIATION_RESPONSE) {
		// should check association response status.
		TRACE ("associated %d", getDestination (packet));
		lookupStation (getDestination (packet))->recordAssociated ();
	}
}

void
MacHighAccessPoint::queueToLow (Packet *packet)
{
	TRACE ("queue %s to %d/%d from %d", 
	       getTypeString (packet), 
	       getDestination (packet), 
	       getFinalDestination (packet), 
	       getSource (packet));
	m_low->enqueue (packet);
}


void 
MacHighAccessPoint::enqueueFromLL (Packet *packet)
{
	setSource (packet, getSelf ());
	setDestination (packet, getFinalDestination (packet));
	setType (packet, MAC_80211_DATA);
	queueToLow (packet);
}

void 
MacHighAccessPoint::receiveFromPhy (Packet *packet)
{
	/* If we had multiple MacLow, we would need 
	 * to choose the right one here.
	 */
	m_low->receive (packet);
}


void 
MacHighAccessPoint::receiveFromMacLow (Packet *packet)
{
	if (getFinalDestination (packet) == getSelf () &&
	    getDestination (packet) == getSelf ()) {
		switch (getType (packet)) {
		case MAC_80211_MGT_BEACON:
		case MAC_80211_MGT_ASSOCIATION_RESPONSE:
		case MAC_80211_MGT_REASSOCIATION_RESPONSE:
		case MAC_80211_MGT_PROBE_RESPONSE:
			/* packets ignored. coming from another AP. */
		case MAC_80211_MGT_DISASSOCIATION:
			/* I have no idea what this frame should be
			 * used for.
			 */
		case MAC_80211_MGT_AUTHENTICATION:
		case MAC_80211_MGT_DEAUTHENTICATION:
			/* packets ignored. We do not perform
			 * any form of authentication so we assume
			 * all stations are always authenticated.
			 */
			Packet::free (packet);
			break;
		case MAC_80211_MGT_ASSOCIATION_REQUEST:
			/* verify that supports all rates in BSS basic rate set. */
			sendAssociationResponseOk (getSource (packet));
			Packet::free (packet);
			break;
		case MAC_80211_MGT_REASSOCIATION_REQUEST:
			/* verify that supports all rates in BSS basic rate set. */
			sendReAssociationResponseOk (getSource (packet));
			Packet::free (packet);
			break;
		case MAC_80211_MGT_PROBE_REQUEST:
			sendProbeResponse (getSource (packet));
			Packet::free (packet);
			break;
		case MAC_80211_DATA:
			TRACE ("forward up from %d", getSource (packet));
			forwardUp (packet);
			break;
		default:
			break;
		}
	} else if (getDestination (packet) == getSelf () &&
		   getFinalDestination (packet) == ((int)MAC_BROADCAST)) {
		TRACE ("forward broadcast from %d", getSource (packet));
		setDestination (packet, getFinalDestination (packet));
		queueToLow (packet->copy ());
		forwardUp (packet);
	} else if (getDestination (packet) == getSelf ()) {
		MacStation *station = lookupStation (getSource (packet));
		if (station->isAssociated ()) {
			// forward to target station.
			TRACE ("%d associated: forwarding packet", getSource (packet));
			enqueueFromLL (packet);
		} else {
			TRACE ("%d not associated: dump packet", getSource (packet));
			Packet::free (packet);
		}
	} else {
		Packet::free (packet);
	}
		
}
