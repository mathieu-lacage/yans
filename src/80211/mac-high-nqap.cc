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

#include "mac-high-nqap.h"
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

#ifndef NQAP_TRACE
#define nopeNQAP_TRACE 1
#endif /* NQAP_TRACE */

#ifdef NQAP_TRACE
# define TRACE(format, ...) \
	printf ("NQAP TRACE %d %f " format "\n", container ()->selfAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* NQAP_TRACE */
# define TRACE(format, ...)
#endif /* NQAP_TRACE */


MacHighNqap::MacHighNqap (MacContainer *container)
	: MacHighAp (container)
{
	m_dcfParameters = new MacDcfParameters (container);
	m_dcf = new Dcf (container, m_dcfParameters);
	m_queue = new MacQueue80211e (container->parameters ());
	new DcaTxop (m_dcf, m_queue, container);

	m_sendBeaconTimer = new StaticHandler<MacHighNqap> (this, &MacHighNqap::sendBeacon);
	m_sendBeaconTimer->start (parameters ()->getBeaconInterval ());
}
MacHighNqap::~MacHighNqap ()
{}

MacParameters *
MacHighNqap::parameters (void)
{
	return container ()->parameters ();
}

void
MacHighNqap::sendBeacon (void)
{
	Packet *packet = getPacketFor (MAC_BROADCAST);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_BEACON));
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
MacHighNqap::getPacketFor (int destination)
{
	Packet *packet = Packet::alloc ();
	setSource (packet, container ()->selfAddress ());
	setFinalDestination (packet, destination);
	setDestination (packet, destination);
	return packet;
}

void
MacHighNqap::sendAssociationResponseOk (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_ASSOCIATION_RESPONSE));
	setType (packet, MAC_80211_MGT_ASSOCIATION_RESPONSE);
	enqueueToLow (packet);
}

void
MacHighNqap::sendReAssociationResponseOk (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_REASSOCIATION_RESPONSE));
	setType (packet, MAC_80211_MGT_REASSOCIATION_RESPONSE);
	enqueueToLow (packet);
}

void
MacHighNqap::sendProbeResponse (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_PROBE_RESPONSE));
	setType (packet, MAC_80211_MGT_PROBE_RESPONSE);
	enqueueToLow (packet);
}

void 
MacHighNqap::gotAddTsRequest (Packet *packet)
{
	assert (false);
}
void 
MacHighNqap::gotDelTsRequest (Packet *packet)
{
	assert (false);
}

void 
MacHighNqap::gotCFPoll (Packet *packet)
{
	assert (false);
}

void
MacHighNqap::forwardQueueToLow (Packet *packet)
{
	enqueueToLow (packet);
}

void
MacHighNqap::enqueueToLow (Packet *packet)
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
MacHighNqap::addTsRequest (TSpecRequest *request)
{
	assert (false);
}
void 
MacHighNqap::delTsRequest (TSpecRequest *request)
{
	assert (false);
}
