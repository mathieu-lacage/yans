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

#include "mac-high-station.h"
#include "mac-80211.h"
#include "arf-mac-stations.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "mac-low.h"
#include "mac-queue-80211e.h"
#include "mac-dcf-parameters.h"
#include "dcf.h"
#include "dca-txop.h"
#include "mac-parameters.h"
#include "mac-container.h"

#include "packet.h"

#define STATION_TRACE 1

#ifdef STATION_TRACE
# define TRACE(format, ...) \
	printf ("STA TRACE %d %f " format "\n", container ()->selfAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* STATION_TRACE */
# define TRACE(format, ...)
#endif /* STATION_TRACE */


MacHighStation::MacHighStation (MacContainer *container, int apAddress)
	: MacHigh (container)
{
	m_dcfParameters = new MacDcfParameters (container);
	m_dcf = new Dcf (container, m_dcfParameters);
	m_dcfQueue = new MacQueue80211e (container->parameters ());
	new DcaTxop (m_dcf, m_dcfQueue, container);

	m_associationQueue = new MacQueue80211e (container->parameters ());

	m_apAddress = apAddress;
	m_associated = false;
	m_lastBeaconRx = 0.0;
}
MacHighStation::~MacHighStation ()
{}

MacParameters *
MacHighStation::parameters (void)
{
	return container ()->parameters ();
}

void
MacHighStation::enqueueToLow (Packet *packet)
{
	m_dcfQueue->enqueue (packet);
	m_dcf->requestAccess ();
}

Packet *
MacHighStation::getPacketFor (int destination)
{
        Packet *packet = Packet::alloc ();
        setSource (packet, container ()->selfAddress ());
        setFinalDestination (packet, destination);
        setDestination (packet, destination);
        return packet;
}


void
MacHighStation::sendAssociationRequest (void)
{
	int destination = m_apAddress;
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getAssociationRequestSize ());
	setType (packet, MAC_80211_MGT_ASSOCIATION_REQUEST);
	enqueueToLow (packet);
}

void
MacHighStation::sendReAssociationRequest (void)
{
	int destination = m_apAddress;
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getReAssociationRequestSize ());
	setType (packet, MAC_80211_MGT_REASSOCIATION_REQUEST);
	enqueueToLow (packet);
}

void
MacHighStation::sendProbeRequest (void)
{
	int destination = m_apAddress;
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getProbeRequestSize ());
	setType (packet, MAC_80211_MGT_PROBE_REQUEST);
	enqueueToLow (packet);
}

void
MacHighStation::notifyAckReceivedFor (Packet *packet)
{}

void
MacHighStation::tryToEnsureAssociated (void)
{
	if (isBeaconMissed ()) {
		TRACE ("missed too many beacons");
		// XXX
		m_dcfQueue->flush ();
		setDisAssociated ();
		sendProbeRequest ();
	} else if (!isAssociated ()) {
		sendAssociationRequest ();
	}
}
void
MacHighStation::emptyAssociationQueue (void)
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
MacHighStation::recordBeaconRx (Packet *packet)
{
	m_lastBeaconRx = now ();
}

bool
MacHighStation::isBeaconMissed (void)
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
MacHighStation::isAssociated (void)
{
	return m_associated;
}
void
MacHighStation::setAssociated (void)
{
	m_associated = true;
}
void
MacHighStation::setDisAssociated (void)
{
	m_associated = false;
}

double
MacHighStation::now (void)
{
	double now;
	now = Scheduler::instance ().clock ();
	return now;
}



void 
MacHighStation::enqueueFromLL (Packet *packet)
{
	tryToEnsureAssociated ();
	setSource (packet, container ()->selfAddress ());
	setDestination (packet, m_apAddress);
	setType (packet, MAC_80211_DATA);
	if (isAssociated ()) {
		TRACE ("send to %d thru %d", getFinalDestination (packet), getDestination (packet));
		enqueueToLow (packet);
	} else {
		TRACE ("temporarily queue");
		m_associationQueue->enqueue (packet);
	}
}

void 
MacHighStation::receiveFromMacLow (Packet *packet)
{
	if (getFinalDestination (packet) == container ()->selfAddress () || 
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
			Packet::free (packet);
			break;
		case MAC_80211_MGT_REASSOCIATION_RESPONSE:
			/* XXX verify status. */
			TRACE ("associated");
			setAssociated ();
			emptyAssociationQueue ();
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
		case MAC_80211_DATA:
			TRACE ("forward up from %d", getSource (packet));
			container ()->forwardToLL (packet);
			break;
		default:
			assert (false);
			break;
		}
	}
}
