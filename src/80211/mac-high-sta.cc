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

#include "mac-high-sta.h"
#include "arf-mac-stations.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "mac-low.h"
#include "mac-queue-80211e.h"
#include "mac-dcf-parameters.h"
#include "dcf.h"
#include "dca-txop.h"
#include "mac-parameters.h"
#include "net-interface-80211.h"
#include "mac-traces.h"
#include "common.h"
#include "ll-arp.h"

#include "packet.h"

#ifndef STATION_TRACE
#define nopeSTATION_TRACE 1
#endif /* STATION_TRACE */

#ifdef STATION_TRACE
# define TRACE(format, ...) \
	printf ("STA TRACE %d %f " format "\n", m_interface->getMacAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* STATION_TRACE */
# define TRACE(format, ...)
#endif /* STATION_TRACE */


MacHighSta::MacHighSta (int apAddress)
	: MacHigh ()
{
	m_associationQueue = new MacQueue80211e ();

	m_apAddress = apAddress;
	m_associated = false;
	m_lastBeaconRx = 0.0;
}
MacHighSta::~MacHighSta ()
{}

void
MacHighSta::notifySetNetInterface (NetInterface80211 *interface)
{
	m_interface = interface;
	m_associationQueue->setParameters (interface->parameters ());
}

MacParameters *
MacHighSta::parameters (void)
{
	return m_interface->parameters ();
}

int 
MacHighSta::getApAddress (void)
{
	return m_apAddress;
}


Packet *
MacHighSta::getPacketFor (int destination)
{
	Packet *packet = hdr_mac_80211::create (m_interface->getMacAddress ());
        setFinalDestination (packet, destination);
        setDestination (packet, m_apAddress);
        return packet;
}


void
MacHighSta::sendAssociationRequest (void)
{
	int destination = m_apAddress;
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_ASSOCIATION_REQUEST));
	setType (packet, MAC_80211_MGT_ASSOCIATION_REQUEST);
	enqueueToLow (packet);
}

void
MacHighSta::sendReAssociationRequest (void)
{
	int destination = m_apAddress;
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_REASSOCIATION_REQUEST));
	setType (packet, MAC_80211_MGT_REASSOCIATION_REQUEST);
	enqueueToLow (packet);
}

void
MacHighSta::sendProbeRequest (void)
{
	int destination = m_apAddress;
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_PROBE_REQUEST));
	setType (packet, MAC_80211_MGT_PROBE_REQUEST);
	enqueueToLow (packet);
}

void
MacHighSta::notifyAckReceivedFor (Packet *packet)
{}

void
MacHighSta::tryToEnsureAssociated (void)
{
	if (isBeaconMissed ()) {
		TRACE ("missed too many beacons");
		// XXX
		flush ();
		setDisAssociated ();
		sendProbeRequest ();
	} else if (!isAssociated ()) {
		sendAssociationRequest ();
	}
}
void
MacHighSta::emptyAssociationQueue (void)
{
	int n = 0;
	Packet *packet;
	packet = m_associationQueue->dequeue ();
	while (packet != NULL) {
		enqueueToLow (packet);
		packet = m_associationQueue->dequeue ();
		n++;
	}
	TRACE ("empty/queue %d", n);
}



void
MacHighSta::recordBeaconRx (Packet *packet)
{
	m_lastBeaconRx = now ();
	gotBeacon (packet);
}

bool
MacHighSta::isBeaconMissed (void)
{
	if ((now () - m_lastBeaconRx) / parameters ()->getBeaconInterval () > parameters ()->getMaxMissedBeacon ()) {
		/* we missed too many beacons
		 */
		return true;
	} else {
		return false;
	}
}

bool
MacHighSta::isAssociated (void)
{
	return m_associated;
}
void
MacHighSta::setAssociated (void)
{
	m_associated = true;
}
void
MacHighSta::setDisAssociated (void)
{
	m_associated = false;
}

double
MacHighSta::now (void)
{
	double now;
	now = Scheduler::instance ().clock ();
	return now;
}

void
MacHighSta::queueToLowDirectly (Packet *packet)
{
	tryToEnsureAssociated ();
	setSource (packet, m_interface->getMacAddress ());
	setDestination (packet, m_apAddress);
	if (isAssociated ()) {
		TRACE ("send to %d thru %d", getFinalDestination (packet), getDestination (packet));
		enqueueToLow (packet);
	} else {
		TRACE ("temporarily queue");
		m_associationQueue->enqueue (packet);
	}
}



void 
MacHighSta::enqueueFromLL (Packet *packet, int macDestination)
{
	setType (packet, MAC_80211_DATA);
	setFinalDestination (packet, macDestination);
	queueToLowDirectly (packet);
}

void 
MacHighSta::receiveFromMacLow (Packet *packet)
{
	if (getFinalDestination (packet) == m_interface->getMacAddress () || 
	    getFinalDestination (packet) == (int)MAC_BROADCAST) {
		switch (getType (packet)) {
		case MAC_80211_MGT_BEACON:
		case MAC_80211_MGT_PROBE_RESPONSE:
			if (getSource (packet) == m_apAddress) {
				recordBeaconRx (packet);
				if (!isAssociated ()) {
					sendAssociationRequest ();
				}
			}
			Packet::free (packet);
			break;
		case MAC_80211_MGT_ASSOCIATION_RESPONSE:
			/* XXX verify status. */
			TRACE ("associated");
			setAssociated ();
			emptyAssociationQueue ();
			gotAssociated (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_REASSOCIATION_RESPONSE:
			/* XXX verify status. */
			TRACE ("associated");
			setAssociated ();
			emptyAssociationQueue ();
			gotReAssociated (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_DISASSOCIATION:
			/* I have no idea what this frame should be
			 * used for and who should be sending it.
			 */
		case MAC_80211_MGT_AUTHENTICATION:
		case MAC_80211_MGT_DEAUTHENTICATION:
			/* packets ignored. We do not perform
			 * any form of authentication so we assume
			 * all stations are always authenticated.
			 */
		case MAC_80211_MGT_ASSOCIATION_REQUEST:
		case MAC_80211_MGT_REASSOCIATION_REQUEST:
		case MAC_80211_MGT_PROBE_REQUEST:
			Packet::free (packet);
			break;
		case MAC_80211_MGT_ADDTS_REQUEST:
		case MAC_80211_MGT_DELTS_REQUEST:
			Packet::free (packet);
			break;
		case MAC_80211_MGT_ADDTS_RESPONSE:
			gotAddTsResponse (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_DELTS_RESPONSE:
			gotDelTsResponse (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_CFPOLL:
			gotCFPoll (packet);
			Packet::free (packet);
			break;
		case MAC_80211_MGT_QOSNULL:
			Packet::free (packet);
			break;
		case MAC_80211_DATA:
			TRACE ("forward up from %d", getSource (packet));
			m_interface->ll ()->sendUp (packet);
			break;
		default:
			assert (false);
			break;
		}
	}
}
