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

#include "mac-high-qap.h"
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
#include "hcca-txop.h"
#include "qap-scheduler.h"
#include "tspec-request.h"
#include "tspec.h"

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


MacHighQap::MacHighQap (MacContainer *container)
	: MacHighAp (container)
{
	createAC (AC_BE);
	createAC (AC_BK);
	createAC (AC_VI);
	createAC (AC_VO);

	m_hcca = new HccaTxop (container);

	m_scheduler = new QapScheduler (container);
}
MacHighQap::~MacHighQap ()
{}

MacParameters *
MacHighQap::parameters (void)
{
	return container ()->parameters ();
}

void
MacHighQap::addEDCAparameters (Packet *packet)
{
	packet->allocdata (4*4);
	unsigned char *buffer = packet->accessdata ();

	// XXX should probably store these in the EDCF
	// scheduler and ask for them here rather
	// than getting them from our own param array.
	for (uint8_t ac = 0; ac < 4; ac++) {
		uint8_t AIFSN = m_staDcfParameters[ac].AIFSN;
		uint8_t ACM = (m_staDcfParameters[ac].ACM)?1:0;
		*buffer = AIFSN | (ACM << 4) | (ac << 5);
		buffer++;
		uint8_t ECWmin = m_staDcfParameters[ac].ECWmin;
		uint8_t ECWmax = m_staDcfParameters[ac].ECWmax;
		*buffer = ECWmin | (ECWmax << 4);
		buffer++;
		uint16_t txop = m_staDcfParameters[ac].txop;
		*buffer = (txop & 0xff);
		buffer++;
		*buffer = (txop >> 8);
		buffer++;
	}
}

void
MacHighQap::createAC (enum ac_e ac)
{
	MacQueue80211e *queue = new MacQueue80211e (container ()->parameters ());
	MacDcfParameters *dcfParameters = new MacDcfParameters (container ());
	Dcf *dcf = new Dcf (container (), dcfParameters);
	new DcaTxop (dcf, queue, container ());

	m_dcfQueues[ac] = queue;
	m_dcfParameters = dcfParameters;
	m_dcfs[ac] = dcf;
}

void
MacHighQap::createTS (TSpec *tspec)
{
	MacQueue80211e *queue = new MacQueue80211e (container ()->parameters ());
	m_ts[tspec->getTSID ()] = make_pair (tspec, queue);
	m_hcca->addStream (queue, tspec->getTSID ());
}
void
MacHighQap::destroyTS (uint8_t tsid)
{
	m_hcca->deleteStream (tsid);
	delete m_ts[tsid].second;
	delete m_ts[tsid].first;
	m_ts[tsid].first = 0;
	m_ts[tsid].second = 0;
}


void
MacHighQap::queueAC (enum ac_e ac, Packet *packet)
{
	m_dcfQueues[ac]->enqueue (packet);
	m_dcfs[ac]->requestAccess ();
}

void
MacHighQap::queueTS (uint8_t tsid, Packet *packet)
{
	assert (isTsActive (tsid));
	m_ts[tsid].second->enqueue (packet);
}

bool
MacHighQap::isTsActive (uint8_t tsid)
{
	assert (tsid >= 8);
	if (m_ts[tsid].first == 0) {
		return false;
	} else {
		return true;
	}
}



void 
MacHighQap::delTsRequest (TSpecRequest *request)
{
	if (m_scheduler->delTsRequest (container ()->selfAddress (), 
				       request->getTSpec ())) {
		request->notifyGranted ();
		destroyTS (request->getTSpec ()->getTSID ());
	} else {
		request->notifyRefused ();
	}
}

void 
MacHighQap::addTsRequest (TSpecRequest *request)
{
	if (m_scheduler->addTsRequest (request->getTSpec ())) {
		request->notifyGranted ();
		createTS (request->getTSpec ());
	} else {
		request->notifyRefused ();
	}
}


Packet *
MacHighQap::getPacketFor (int destination)
{
	Packet *packet = Packet::alloc ();
	setSource (packet, container ()->selfAddress ());
	setFinalDestination (packet, destination);
	setDestination (packet, destination);
	return packet;
}

void
MacHighQap::sendAssociationResponseOk (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_ASSOCIATION_RESPONSE));
	setType (packet, MAC_80211_MGT_ASSOCIATION_RESPONSE);
	addEDCAparameters (packet);
	queueAC (AC_VO, packet);
}

void
MacHighQap::sendReAssociationResponseOk (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_REASSOCIATION_RESPONSE));
	setType (packet, MAC_80211_MGT_REASSOCIATION_RESPONSE);
	addEDCAparameters (packet);
	queueAC (AC_VO, packet);
}

void
MacHighQap::sendProbeResponse (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_PROBE_RESPONSE));
	setType (packet, MAC_80211_MGT_PROBE_RESPONSE);
	queueAC (AC_VO, packet);
}

void
MacHighQap::queueTsResponse (int destination, TSpecRequest *request,
			     enum mac_80211_packet_type type, 
			     enum mac_80211_request_status status)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (type));
	setType (packet, type);
	packet->allocdata (sizeof (status)+sizeof (request));
	uint8_t *buffer = packet->accessdata ();
	*((enum mac_80211_request_status *)buffer) = status;
	buffer += sizeof (status);
	*((TSpecRequest **)buffer) = request;
	queueAC (AC_VO, packet);
}

void
MacHighQap::queueAddTsResponseOk (int destination, 
				  TSpecRequest *request)
{
	queueTsResponse (destination, request, 
			 MAC_80211_MGT_ADDTS_RESPONSE, 
			 MAC_80211_ADDTS_OK);
}

void
MacHighQap::queueAddTsResponseRefused (int destination, 
				       TSpecRequest *request)
{
	queueTsResponse (destination, request, 
			 MAC_80211_MGT_ADDTS_RESPONSE, 
			 MAC_80211_ADDTS_FAILED);

}

void
MacHighQap::queueDelTsResponseOk (int destination, 
				  TSpecRequest *request)
{
	queueTsResponse (destination, request, 
			 MAC_80211_MGT_DELTS_RESPONSE, 
			 MAC_80211_ADDTS_OK);

}

void
MacHighQap::queueDelTsResponseRefused (int destination, 
				       TSpecRequest *request)
{
	queueTsResponse (destination, request, 
			 MAC_80211_MGT_DELTS_RESPONSE, 
			 MAC_80211_ADDTS_FAILED);
}

void 
MacHighQap::gotAddTsRequest (Packet *packet)
{
	/* request send by a STA */
	TSpecRequest *request = reinterpret_cast<TSpecRequest *> (packet->accessdata ());
	TSpec *tspec = request->getTSpec ();
	if (m_scheduler->addTsRequest (tspec)) {
		queueAddTsResponseOk (getSource (packet), request);
	} else {
		queueAddTsResponseRefused (getSource (packet), request);
	}	
}

void 
MacHighQap::gotDelTsRequest (Packet *packet)
{
	/* request send by a STA */
	TSpecRequest *request = reinterpret_cast<TSpecRequest *> (packet->accessdata ());
	TSpec *tspec = request->getTSpec ();
	if (m_scheduler->delTsRequest (getSource (packet), tspec)) {
		queueDelTsResponseOk (getSource (packet), request);
	} else {
		queueDelTsResponseRefused (getSource (packet), request);
	}	
}

void 
MacHighQap::gotCFPoll (Packet *packet)
{
	// XXX not implemented
	assert (false);
}


void
MacHighQap::forwardQueueToLow (Packet *packet)
{
	int requestedTID = getRequestedTID (packet);
	if (requestedTID < 8) {
		/* XXX we assume that there is no form of
		 * Access Control for any Access Category.
		 * Of course, this is wrong but we can
		 * implement it later.
		 */
		setTID (packet, requestedTID);
		queueAC (getAC (packet), packet);
	} else {
		if (isTsActive (requestedTID)) {
			setTID (packet, requestedTID);
			queueTS (requestedTID, packet);
		} else {
			// XXX if we have not created this TS
			// yet, we should probably drop this packet.
		}
	}
}

void
MacHighQap::enqueueToLow (Packet *packet)
{
	/* There is a magic -1 here. It is here because in
	 * tspec-request.cc:notifyRequestGranted, we have
	 * a +1. The + and - one are there because we want to
	 * allow the prio IPv6 field to be zero if it has never
	 * been set or non-zero in which case it is supposed to
	 * indicate the number of the target TID+1. 
	 * This is really quite tricky.
	 */
	int requestedTID = getRequestedTID (packet);
	if (requestedTID == 0) {
		/* unmarked so we assume AC_BE unless we have
		 * a management frame in which case it should be
		 * AC_VO. see 9.1.3.1 802.11e/D12.1
		 */
		if (isManagement (packet)) {
			queueAC (AC_VO, packet);
		} else {
			queueAC (AC_BE, packet);
		}
		return;
	} else {
		requestedTID --;
	}
	if (requestedTID < 8) {
		/* XXX we assume that there is no form of
		 * Access Control for any Access Category.
		 * Of course, this is wrong but we can
		 * implement it later.
		 */
		setTID (packet, requestedTID);
		queueAC (getAC (packet), packet);
	} else {
		if (isTsActive (requestedTID)) {
			setTID (packet, requestedTID);
			queueTS (requestedTID, packet);
		} else {
			// XXX if we have not created this TS
			// yet, we should probably drop this packet.
		}
	}
}
