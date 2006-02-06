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

#include "mac-high-qap.h"
#include "arf-mac-stations.h"
#include "mac-stations.h"
#include "mac-station.h"
#include "mac-parameters.h"
#include "mac-dcf-parameters.h"
#include "dcf.h"
#include "edca-txop.h"
#include "mac-queue-80211e.h"
#include "hdr-mac-80211.h"
#include "mac-traces.h"
#include "hcca-txop.h"
#include "qap-scheduler.h"
#include "tspec-request.h"
#include "tspec.h"
#include "net-interface-80211.h"

#include "packet.h"

#ifndef QAP_TRACE
#define QAP_TRACE 1
#endif /* QAP_TRACE */

#ifdef QAP_TRACE
# define TRACE(format, ...) \
	printf ("QAP TRACE %d %f " format "\n", m_interface->getMacAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* QAP_TRACE */
# define TRACE(format, ...)
#endif /* QAP_TRACE */


MacHighQap::MacHighQap ()
	: MacHighAp ()
{
	m_scheduler = new QapScheduler ();

	m_hcca = new HccaTxop ();
}
MacHighQap::~MacHighQap ()
{}

void
MacHighQap::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
	m_scheduler->setInterface (interface);
	m_hcca->setInterface (interface);
	createAC (AC_BE, interface);
	createAC (AC_BK, interface);
	createAC (AC_VI, interface);
	createAC (AC_VO, interface);
}

MacParameters *
MacHighQap::parameters (void)
{
	return m_interface->parameters ();
}

void
MacHighQap::createAC (enum ac_e ac, NetInterface80211 *interface)
{
	MacQueue80211e *queue = new MacQueue80211e ();
	queue->setParameters (interface->parameters ());
	Dcf *dcf = m_scheduler->createDcf (ac);
	EdcaTxop *edcaTxop = new EdcaTxop (dcf, queue);
	edcaTxop->setInterface (m_interface);

	m_dcfQueues[ac] = queue;
	m_dcfs[ac] = dcf;
}


/*
  start of code copied from MacHighQStation
 */

void
MacHighQap::createTS (TSpec *tspec)
{
	MacQueue80211e *queue = new MacQueue80211e ();
	queue->setParameters (parameters ());
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
	TRACE ("queue %s to %d thru %s", 
	       getTypeString (packet), 
	       getDestination (packet),
	       getAcString (packet));
	m_dcfQueues[ac]->enqueue (packet);
	m_dcfs[ac]->requestAccess ();
}

void
MacHighQap::queueTS (uint8_t tsid, Packet *packet)
{
	TRACE ("queue %s to %d thru tsid %d", 
	       getTypeString (packet), 
	       getDestination (packet),
	       getTID (packet));
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

NetInterface80211 *
MacHighQap::interface (void)
{
	return m_interface;
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
		enum ac_e ac;
		if (isManagement (packet)) {
			ac = AC_VO;
		} else {
			ac = AC_BE;
		}
		setAC (packet, ac);
		queueAC (ac, packet);
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
			assert (false);
		}
	}
}

/*
  end of code copied from MacHighQStation
 */


void 
MacHighQap::delTsRequest (TSpecRequest *request)
{
	if (m_scheduler->delTsRequest (m_interface->getMacAddress (), 
				       request->getTSpec ())) {
		destroyTS (request->getTSpec ()->getTSID ());
		request->notifyGranted ();
	} else {
		request->notifyRefused ();
	}
}

void 
MacHighQap::addTsRequest (TSpecRequest *request)
{
	if (m_scheduler->addTsRequest (m_interface->getMacAddress (), request->getTSpec ())) {
		createTS (request->getTSpec ());
		request->notifyGranted ();
	} else {
		request->notifyRefused ();
	}
}


Packet *
MacHighQap::getPacketFor (int destination)
{
	Packet *packet = hdr_mac_80211::create (m_interface->getMacAddress ());
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
	m_scheduler->storeEdcaParametersInPacket (packet);
	setAC (packet, AC_VO);
	queueAC (AC_VO, packet);
}

void
MacHighQap::sendReAssociationResponseOk (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_REASSOCIATION_RESPONSE));
	setType (packet, MAC_80211_MGT_REASSOCIATION_RESPONSE);
	m_scheduler->storeEdcaParametersInPacket (packet);
	setAC (packet, AC_VO);
	queueAC (AC_VO, packet);
}

void
MacHighQap::sendProbeResponse (int destination)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_PROBE_RESPONSE));
	setType (packet, MAC_80211_MGT_PROBE_RESPONSE);
	setAC (packet, AC_VO);
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
	setAC (packet, AC_VO);
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
	TSpecRequest *request = *(reinterpret_cast<TSpecRequest **> (packet->accessdata ()));
	TSpec *tspec = request->getTSpec ();
	if (m_scheduler->addTsRequest (getSource (packet), tspec)) {
		TRACE ("accept tspec for tsid %u from %u", tspec->getTSID (), getSource (packet));
		queueAddTsResponseOk (getSource (packet), request);
	} else {
		TRACE ("refuse tspec for tsid %u from %u", tspec->getTSID (), getSource (packet));
		queueAddTsResponseRefused (getSource (packet), request);
	}	
}

void 
MacHighQap::gotDelTsRequest (Packet *packet)
{
	/* request send by a STA */
	TSpecRequest *request = *(reinterpret_cast<TSpecRequest **> (packet->accessdata ()));
	TSpec *tspec = request->getTSpec ();
	if (m_scheduler->delTsRequest (getSource (packet), tspec)) {
		TRACE ("accept del tspec for tsid %u from %u", tspec->getTSID (), getSource (packet));
		queueDelTsResponseOk (getSource (packet), request);
	} else {
		TRACE ("refuse del tspec for tsid %u from %u", tspec->getTSID (), getSource (packet));
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
MacHighQap::gotQosNull (Packet *packet)
{
	m_scheduler->gotQosNull (packet);
}


void
MacHighQap::forwardQueueToLow (Packet *packet)
{
	/* packets forwarded from a station A to B cannot have
	 * any special priority so we do not give them any.
	 * This is exactly where a TCLAS would be handy.
	 */
	enum ac_e ac;
	if (isManagement (packet)) {
		ac = AC_VO;
	} else {
		ac = AC_BE;
	}
	setAC (packet, ac);
	queueAC (ac, packet);
}
