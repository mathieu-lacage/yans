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

#include "packet.h"

#include "mac-high-qstation.h"
#include "mac-traces.h"
#include "dcf.h"
#include "mac-dcf-parameters.h"
#include "mac-queue-80211e.h"
#include "mac-container.h"
#include "dca-txop.h"
#include "hcca-txop.h"
#include "hdr-mac-80211.h"
#include "tspec-request.h"
#include "tspec.h"


#ifndef QSTATION_TRACE
#define QSTATION_TRACE 1
#endif /* QSTATION_TRACE */

#ifdef QSTATION_TRACE
# define TRACE(format, ...) \
	printf ("QSTA TRACE %d %f " format "\n", container ()->selfAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* QSTATION_TRACE */
# define TRACE(format, ...)
#endif /* QSTATION_TRACE */


MacHighQStation::MacHighQStation (MacContainer *container, int apAddress)
	: MacHighStation (container, apAddress)
{
	createAC (AC_BE);
	createAC (AC_BK);
	createAC (AC_VI);
	createAC (AC_VO);

	m_hcca = new HccaTxop (container);
}
MacHighQStation::~MacHighQStation ()
{}

void
MacHighQStation::createAC (enum ac_e ac)
{
	MacQueue80211e *queue = new MacQueue80211e (container ()->parameters ());
	MacDcfParameters *dcfParameters = new MacDcfParameters (container ());
	Dcf *dcf = new Dcf (container (), dcfParameters);
	new DcaTxop (dcf, queue, container ());

	m_dcfQueues[ac] = queue;
	m_dcfParameters[ac] = dcfParameters;
	m_dcfs[ac] = dcf;
}

void
MacHighQStation::createTS (TSpec *tspec)
{
	MacQueue80211e *queue = new MacQueue80211e (container ()->parameters ());
	m_ts[tspec->getTSID ()] = make_pair (tspec, queue);
	m_hcca->addStream (queue, tspec->getTSID ());
}



void
MacHighQStation::updateEDCAParameters (unsigned char const *buffer)
{
	if (buffer == 0) {
		return;
	}
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t AIFSN = *buffer & 0x0f;
		uint8_t ACM = (*buffer >> 4) & 0x1;
		uint8_t ACI = (*buffer >> 5) & 0x3;
		buffer++;
		uint8_t ECWmin = (*buffer & 0x0f);
		uint8_t ECWmax = (*buffer >> 4) & 0x0f;
		uint16_t CWmin = (1<<ECWmin)-1;
		uint16_t CWmax = (1<<ECWmax)-1;
		buffer++;
		uint16_t txop = buffer[0] | (buffer[1]<<8);
		double txopLimit = txop * 32e-6;
		buffer += 2;
		enum ac_e ac = (enum ac_e) ACI;
		m_dcfParameters[ac]->setCWmin (CWmin);
		m_dcfParameters[ac]->setCWmax (CWmax);
		m_dcfParameters[ac]->setTxopLimit (txopLimit);
		m_dcfParameters[ac]->setAIFSN (AIFSN);
		m_dcfParameters[ac]->setACM (ACM);
	}
}

void
MacHighQStation::queueAC (enum ac_e ac, Packet *packet)
{
	m_dcfQueues[ac]->enqueue (packet);
	m_dcfs[ac]->requestAccess ();
}

void 
MacHighQStation::delTsRequest (TSpecRequest *request)
{
	Packet *addts = getPacketFor (getApAddress ());
	setType (addts, MAC_80211_MGT_ADDTS_REQUEST);
	addts->allocdata (sizeof (request));
	unsigned char *buffer = addts->accessdata ();
	*((TSpecRequest **)buffer) = request;
	// XXX for now, send through AC_BE queue.
	queueAC (AC_BE, addts);
}
void 
MacHighQStation::addTsRequest (TSpecRequest *request)
{
	Packet *addts = getPacketFor (getApAddress ());
	setType (addts, MAC_80211_MGT_ADDTS_REQUEST);
	addts->allocdata (sizeof (request));
	unsigned char *buffer = addts->accessdata ();
	*((TSpecRequest **)buffer) = request;
	// XXX for now, send through AC_BE queue.
	queueAC (AC_BE, addts);
}

bool
MacHighQStation::isTsActive (uint8_t tsid)
{
	assert (tsid >= 8);
	if (m_ts[tsid].first == 0) {
		return false;
	} else {
		return true;
	}
}

void
MacHighQStation::queueTS (uint8_t tsid, Packet *packet)
{
	assert (isTsActive (tsid));
	m_ts[tsid].second->enqueue (packet);
}

void 
MacHighQStation::enqueueToLow (Packet *packet)
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

void 
MacHighQStation::gotCFPoll (Packet *packet)
{
	if (getTID (packet) > 8) {
		m_hcca->tsAccessGranted (getTID (packet), getTxopLimit (packet));
	} else {
		m_hcca->acAccessGranted (getAC (packet), getTxopLimit (packet));
	}
}

void
MacHighQStation::gotReAssociated (Packet *packet)
{
	updateEDCAParameters (packet->accessdata ());
}
void
MacHighQStation::gotAssociated (Packet *packet)
{
	updateEDCAParameters (packet->accessdata ());
}
void
MacHighQStation::gotBeacon (Packet *packet)
{
	updateEDCAParameters (packet->accessdata ());
}
void 
MacHighQStation::gotAddTsResponse (Packet *packet)
{
	uint8_t *buffer = packet->accessdata ();
	enum mac_80211_request_status status = *((enum mac_80211_request_status *)buffer);
	buffer += sizeof (status);
	TSpecRequest *request = *((TSpecRequest **)buffer);
	if (status == MAC_80211_ADDTS_OK) {
		TRACE ("granted TS");
		createTS (request->getTSpec ());
		request->notifyGranted ();
	} else {
		assert (status == MAC_80211_ADDTS_FAILED);
		TRACE ("refused TS");
		request->notifyRefused ();
	}
}
void 
MacHighQStation::gotDelTsResponse (Packet *packet)
{
	uint8_t *buffer = packet->accessdata ();
	enum mac_80211_request_status status = *((enum mac_80211_request_status *)buffer);
	buffer += sizeof (status);
	TSpecRequest *request = *((TSpecRequest **)buffer);
	if (status == MAC_80211_DELTS_OK) {
		TRACE ("deleted TS");
		createTS (request->getTSpec ());
		request->notifyGranted ();
	} else {
		assert (status == MAC_80211_DELTS_FAILED);
		TRACE ("refused deleting TS");
		request->notifyRefused ();
	}
}

void 
MacHighQStation::flush (void)
{
	// XXX
}
