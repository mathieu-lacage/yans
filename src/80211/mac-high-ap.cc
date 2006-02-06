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
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "common.h"
#include "mac-high-ap.h"
#include "net-interface-80211.h"
#include "hdr-mac-80211.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "ll-arp.h"

#include "packet.h"

#ifndef AP_TRACE
#define nopeAP_TRACE 1
#endif /* AP_TRACE */

#ifdef AP_TRACE
# define TRACE(format, ...) \
	printf ("AP TRACE %d %f " format "\n", interface ()->getMacAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* AP_TRACE */
# define TRACE(format, ...)
#endif /* AP_TRACE */


MacHighAp::MacHighAp ()
	: MacHigh ()
{}
MacHighAp::~MacHighAp ()
{}

void 
MacHighAp::enqueueFromLL (Packet *packet, int macDestination)
{
	setSource (packet, interface ()->getMacAddress ());
	setFinalDestination (packet, macDestination);
	setDestination (packet, macDestination);
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
		interface ()->stations ()->lookup (getDestination (packet))->recordAssociated ();
	}
}

void 
MacHighAp::receiveFromMacLow (Packet *packet)
{
	MacStation *station = interface ()->stations ()->lookup (getSource (packet));

	if (getFinalDestination (packet) == interface ()->getMacAddress () &&
	    getDestination (packet) == interface ()->getMacAddress ()) {
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
			if (station->isAssociated ()) {
				gotAddTsRequest (packet);
			}
			Packet::free (packet);
			break;
		case MAC_80211_MGT_DELTS_REQUEST:
			if (station->isAssociated ()) {
				gotDelTsRequest (packet);
			}
			Packet::free (packet);
			break;
		case MAC_80211_MGT_ADDTS_RESPONSE:
		case MAC_80211_MGT_DELTS_RESPONSE:
			Packet::free (packet);
			break;
		case MAC_80211_MGT_CFPOLL:
			gotCFPoll (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_QOSNULL:
			gotQosNull (packet);
			Packet::free (packet);
			break;
		case MAC_80211_DATA:
			if (station->isAssociated ()) {
				TRACE ("forward up from %d", getSource (packet));
				interface ()->ll ()->sendUp (packet);
			} else {
				goto drop;
			}
			break;
		default:
			break;
		}
	} else if (getDestination (packet) == interface ()->getMacAddress () &&
		   getFinalDestination (packet) == ((int)MAC_BROADCAST)) {
		if (station->isAssociated ()) {
			TRACE ("forward broadcast from %d", getSource (packet));
			setDestination (packet, getFinalDestination (packet));
			setSource (packet, interface ()->getMacAddress ());
			forwardQueueToLow (packet->copy ());
			interface ()->ll ()->sendUp (packet);
		} else {
			goto drop;
		}
	} else if (getDestination (packet) == interface ()->getMacAddress ()) {
		MacStation *station = interface ()->stations ()->lookup (getSource (packet));
		if (station->isAssociated ()) {
			// forward to target station.
			TRACE ("%d associated: forwarding packet", getSource (packet));
			setDestination (packet, getFinalDestination (packet));
			setSource (packet, interface ()->getMacAddress ());
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
