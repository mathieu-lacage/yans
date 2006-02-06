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
#include "mac-parameters.h"
#include "mac-dcf-parameters.h"
#include "dcf.h"
#include "dca-txop.h"
#include "mac-container.h"
#include "mac-queue-80211e.h"
#include "hdr-mac-80211.h"
#include "mac-traces.h"

#include "packet.h"

#ifndef AP_TRACE
#define AP_TRACE 1
#endif /* AP_TRACE */

#ifdef AP_TRACE
# define TRACE(format, ...) \
	printf ("AP TRACE %d %f " format "\n", container ()->selfAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* AP_TRACE */
# define TRACE(format, ...)
#endif /* AP_TRACE */


MacHighAccessPoint::MacHighAccessPoint (MacContainer *container)
	: MacHigh (container)
{
	m_dcfParameters = new MacDcfParameters (container);
	m_dcf = new Dcf (container, m_dcfParameters);
	m_queue = new MacQueue80211e (container->parameters ());
	new DcaTxop (m_dcf, m_queue, container);

	m_sendBeaconTimer = new StaticHandler<MacHighAccessPoint> (this, &MacHighAccessPoint::sendBeacon);
	m_sendBeaconTimer->start (parameters ()->getBeaconInterval ());
}
MacHighAccessPoint::~MacHighAccessPoint ()
{}

MacParameters *
MacHighAccessPoint::parameters (void)
{
	return container ()->parameters ();
}

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
	m_queue->enqueueToHead (packet);
	m_dcf->requestAccess ();

	m_sendBeaconTimer->start (parameters ()->getBeaconInterval ());
}

Packet *
MacHighAccessPoint::getPacketFor (int destination)
{
	Packet *packet = Packet::alloc ();
	setSource (packet, container ()->selfAddress ());
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
		container ()->stations ()->lookup (getDestination (packet))->recordAssociated ();
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
	m_queue->enqueue (packet);
	m_dcf->requestAccess ();
}


void 
MacHighAccessPoint::enqueueFromLL (Packet *packet)
{
	setSource (packet, container ()->selfAddress ());
	setDestination (packet, getFinalDestination (packet));
	setType (packet, MAC_80211_DATA);
	queueToLow (packet);
}

void 
MacHighAccessPoint::receiveFromMacLow (Packet *packet)
{
	if (getFinalDestination (packet) == container ()->selfAddress () &&
	    getDestination (packet) == container ()->selfAddress ()) {
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
			container ()->forwardToLL (packet);
			break;
		default:
			break;
		}
	} else if (getDestination (packet) == container ()->selfAddress () &&
		   getFinalDestination (packet) == ((int)MAC_BROADCAST)) {
		TRACE ("forward broadcast from %d", getSource (packet));
		setDestination (packet, getFinalDestination (packet));
		queueToLow (packet->copy ());
		container ()->forwardToLL (packet);
	} else if (getDestination (packet) == container ()->selfAddress ()) {
		MacStation *station = container ()->stations ()->lookup (getSource (packet));
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
