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

#include "mac.h"

#include "mac-high-ap.h"
#include "mac-container.h"
#include "packet.h"
#include "hdr-mac-80211.h"
#include "mac-stations.h"
#include "mac-station.h"

#ifndef AP_TRACE
#define nopeAP_TRACE 1
#endif /* AP_TRACE */

#ifdef AP_TRACE
# define TRACE(format, ...) \
	printf ("AP TRACE %d %f " format "\n", container ()->selfAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* AP_TRACE */
# define TRACE(format, ...)
#endif /* AP_TRACE */


MacHighAp::MacHighAp (MacContainer *container)
	: MacHigh (container)
{}
MacHighAp::~MacHighAp ()
{}

void 
MacHighAp::enqueueFromLL (Packet *packet)
{
	setSource (packet, container ()->selfAddress ());
	setDestination (packet, getFinalDestination (packet));
	setType (packet, MAC_80211_DATA);
	enqueueToLow (packet);
}

void
MacHighAp::notifyAckReceivedFor (Packet *packet)
{
	if (getType (packet) == MAC_80211_MGT_ASSOCIATION_RESPONSE ||
	    getType (packet) == MAC_80211_MGT_REASSOCIATION_RESPONSE) {
		// should check association response status.
		TRACE ("associated %d", getDestination (packet));
		container ()->stations ()->lookup (getDestination (packet))->recordAssociated ();
	}
}

void 
MacHighAp::receiveFromMacLow (Packet *packet)
{
	MacStation *station = container ()->stations ()->lookup (getSource (packet));

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
		case MAC_80211_MGT_ADDTS_REQUEST:
			gotAddTsRequest (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_DELTS_REQUEST:
			gotDelTsRequest (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_ADDTS_RESPONSE:
		case MAC_80211_MGT_DELTS_RESPONSE:
			Packet::free (packet);
			break;
		case MAC_80211_MGT_CFPOLL:
			break;
			gotCFPoll (packet);
		case MAC_80211_DATA:
			if (station->isAssociated ()) {
				TRACE ("forward up from %d", getSource (packet));
				container ()->forwardToLL (packet);
			} else {
				goto drop;
			}
			break;
		default:
			break;
		}
	} else if (getDestination (packet) == container ()->selfAddress () &&
		   getFinalDestination (packet) == ((int)MAC_BROADCAST)) {
		if (station->isAssociated ()) {
			TRACE ("forward broadcast from %d", getSource (packet));
			setDestination (packet, getFinalDestination (packet));
			setSource (packet, container ()->selfAddress ());
			forwardQueueToLow (packet->copy ());
			container ()->forwardToLL (packet);
		} else {
			goto drop;
		}
	} else if (getDestination (packet) == container ()->selfAddress ()) {
		MacStation *station = container ()->stations ()->lookup (getSource (packet));
		if (station->isAssociated ()) {
			// forward to target station.
			TRACE ("%d associated: forwarding packet", getSource (packet));
			setDestination (packet, getFinalDestination (packet));
			setSource (packet, container ()->selfAddress ());
			forwardQueueToLow (packet);
		} else {
			goto drop;
		}
	} else {
		Packet::free (packet);
	}

	return;
 drop:
	TRACE ("%d not associated: dump packet", getSource (packet));
	Packet::free (packet);
}
