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

#include "packet.h"

#include "mac-high-qsta.h"
#include "mac-traces.h"
#include "dcf.h"
#include "mac-dcf-parameters.h"
#include "mac-queue-80211e.h"
#include "net-interface-80211.h"
#include "edca-txop.h"
#include "hcca-txop.h"
#include "hdr-mac-80211.h"
#include "tspec-request.h"
#include "tspec.h"


#ifndef QSTATION_TRACE
#define QSTATION_TRACE 1
#endif /* QSTATION_TRACE */

#ifdef QSTATION_TRACE
# define TRACE(format, ...) \
	printf ("QSTA TRACE %d %f " format "\n", m_interface->getMacAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* QSTATION_TRACE */
# define TRACE(format, ...)
#endif /* QSTATION_TRACE */


MacHighQsta::MacHighQsta (int apAddress)
	: MacHighSta (apAddress)
{
	m_hcca = new HccaTxop ();
}
MacHighQsta::~MacHighQsta ()
{}

void
MacHighQsta::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
	MacHighSta::notifySetNetInterface (interface);

	m_hcca->setInterface (interface);

	createAC (AC_BE, interface);
	createAC (AC_BK, interface);
	createAC (AC_VI, interface);
	createAC (AC_VO, interface);
}

void
MacHighQsta::createAC (enum ac_e ac, NetInterface80211 *interface)
{
	MacQueue80211e *queue = new MacQueue80211e ();
	queue->setParameters (interface->parameters ());
	MacDcfParameters *dcfParameters = new MacDcfParameters ();
	dcfParameters->setParameters (interface->parameters ());
	Dcf *dcf = new Dcf (dcfParameters);
	dcf->setInterface (interface);
	EdcaTxop *edcaTxop = new EdcaTxop (dcf, queue);
	edcaTxop->setInterface (interface);

	m_dcfQueues[ac] = queue;
	m_dcfParameters[ac] = dcfParameters;
	m_dcfs[ac] = dcf;
}

void
MacHighQsta::createTS (TSpec *tspec)
{
	MacQueue80211e *queue = new MacQueue80211e ();
	queue->setParameters (m_interface->parameters ());
	m_ts[tspec->getTSID ()] = make_pair (tspec, queue);
	m_hcca->addStream (queue, tspec->getTSID ());
}



void
MacHighQsta::updateEDCAParameters (unsigned char const *buffer)
{
	if (buffer == 0) {
		return;
	}
	for (uint8_t i = 0; i < 4; i++) {
		m_dcfParameters[i]->readFrom (buffer);
		buffer += 4;
	}
}

void
MacHighQsta::queueAC (enum ac_e ac, Packet *packet)
{
	TRACE ("queue \"%s\" to %d thru %s", 
	       getTypeString (packet), 
	       getDestination (packet),
	       getAcString (packet));
	m_dcfQueues[ac]->enqueue (packet);
	m_dcfs[ac]->requestAccess ();
}

void 
MacHighQsta::delTsRequest (TSpecRequest *request)
{
	Packet *delts = getPacketFor (getApAddress ());
	setType (delts, MAC_80211_MGT_DELTS_REQUEST);
	delts->allocdata (sizeof (request));
	unsigned char *buffer = delts->accessdata ();
	*((TSpecRequest **)buffer) = request;
	// XXX for now, send through AC_BE queue.
	setAC (delts, AC_BE);
	queueToLowDirectly (delts);
}
void 
MacHighQsta::addTsRequest (TSpecRequest *request)
{
	TSpec *tspec = request->getTSpec ();
	bool found = false;
	for (int i = 8; i < 16; i++) {
		if (m_ts.find (i) == m_ts.end ()) {
			/* this is an available tsid. */
			tspec->setTsid (i);
			found = true;
			break;
		}
	}
	if (!found) {
		TRACE ("no tsids available");
		request->notifyRefused ();
		return;
	}
	if (tspec->getMinimumServiceInterval () == 0.0) {
		if (tspec->getNominalMSDUSize () == 0 &&
		    tspec->getMeanDataRate () == 0.0) {
			TRACE ("invalid tspec: min SI not specified and cannot be calculated");
			request->notifyRefused ();
			return;
		}
		tspec->setMinimumServiceInterval (tspec->getNominalMSDUSize () / tspec->getMeanDataRate ());
	}
	if (tspec->getMaximumServiceInterval () == 0.0) {
		if (tspec->getDelayBound () == 0.0) {
			TRACE ("invalid tspec: max SI not specified and cannot be calculated");
			request->notifyRefused ();
			return;			
		}
		uint8_t maxRetriesPossible = 1;
		tspec->setMaximumServiceInterval (tspec->getDelayBound () / maxRetriesPossible);
	}
	if (tspec->getMinimumServiceInterval () > tspec->getMaximumServiceInterval ()) {
		TRACE ("invalid tspec: min SI (%f) > max SI (%f)",
		       tspec->getMinimumServiceInterval (),
		       tspec->getMaximumServiceInterval ());
		request->notifyRefused ();
		return;
	}
	Packet *addts = getPacketFor (getApAddress ());
	setType (addts, MAC_80211_MGT_ADDTS_REQUEST);
	addts->allocdata (sizeof (request));
	unsigned char *buffer = addts->accessdata ();
	*((TSpecRequest **)buffer) = request;
	// XXX for now, send through AC_BE queue.
	setAC (addts, AC_BE);
	queueToLowDirectly (addts);
}

bool
MacHighQsta::isTsActive (uint8_t tsid)
{
	assert (tsid >= 8);
	if (m_ts[tsid].first == 0) {
		return false;
	} else {
		return true;
	}
}

void
MacHighQsta::queueTS (uint8_t tsid, Packet *packet)
{
	TRACE ("queue %s to %d thru tsid %d", 
	       getTypeString (packet), 
	       getDestination (packet),
	       getTID (packet));
	assert (isTsActive (tsid));
	m_ts[tsid].second->enqueue (packet);
}

void 
MacHighQsta::enqueueToLow (Packet *packet)
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
	TRACE ("requested tid %d", requestedTID);
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

void 
MacHighQsta::gotCFPoll (Packet *packet)
{
	TRACE ("got cfpoll %d/%f", getTID (packet), getTxopLimit (packet));
	if (getTID (packet) < 8) {
		m_hcca->acAccessGranted (getAC (packet), getTxopLimit (packet));
	} else {
		m_hcca->tsAccessGranted (getTID (packet), getTxopLimit (packet));
	}
}

void
MacHighQsta::gotReAssociated (Packet *packet)
{
	updateEDCAParameters (packet->accessdata ());
}
void
MacHighQsta::gotAssociated (Packet *packet)
{
	updateEDCAParameters (packet->accessdata ());
}
void
MacHighQsta::gotBeacon (Packet *packet)
{
	updateEDCAParameters (packet->accessdata ());
}
void 
MacHighQsta::gotAddTsResponse (Packet *packet)
{
	uint8_t *buffer = packet->accessdata ();
	enum mac_80211_request_status status = *((enum mac_80211_request_status *)buffer);
	buffer += sizeof (status);
	TSpecRequest *request = *((TSpecRequest **)buffer);
	TSpec *tspec = request->getTSpec ();
	if (status == MAC_80211_ADDTS_OK) {
		TRACE ("granted TS for tsid %d", tspec->getTSID ());
		createTS (tspec);
		request->notifyGranted ();
	} else {
		assert (status == MAC_80211_ADDTS_FAILED);
		TRACE ("refused TS for tsid %d", tspec->getTSID ());
		request->notifyRefused ();
	}
}
void 
MacHighQsta::gotDelTsResponse (Packet *packet)
{
	uint8_t *buffer = packet->accessdata ();
	enum mac_80211_request_status status = *((enum mac_80211_request_status *)buffer);
	buffer += sizeof (status);
	TSpecRequest *request = *((TSpecRequest **)buffer);
	TSpec *tspec = request->getTSpec ();
	if (status == MAC_80211_DELTS_OK) {
		TRACE ("deleted TS for tsid %d", tspec->getTSID ());
		createTS (tspec);
		request->notifyGranted ();
	} else {
		assert (status == MAC_80211_DELTS_FAILED);
		TRACE ("refused deleting TS for tsid %d", tspec->getTSID ());
		request->notifyRefused ();
	}
}

void 
MacHighQsta::flush (void)
{
	// XXX
}
