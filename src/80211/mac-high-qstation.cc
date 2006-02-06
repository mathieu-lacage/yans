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
	Dcf *dcf;
	MacQueue80211e *queue;
	MacDcfParameters *dcfParameters;

	queue = new MacQueue80211e (container->parameters ());
	dcfParameters = new MacDcfParameters (container);
	dcf = new Dcf (container, dcfParameters);
	new DcaTxop (dcf, queue, container);

	m_hcca = new HccaTxop (container);
}
MacHighQStation::~MacHighQStation ()
{}

bool
MacHighQStation::isAcActive (uint8_t UP)
{
	// XXX
	return true;
}

bool
MacHighQStation::isStreamActive (uint8_t TSID)
{
	// XXX
	return true;
}

void 
MacHighQStation::enqueueToLow (Packet *packet)
{
	int requestedTID = getRequestedTID (packet);
	if (requestedTID < 8) {
		if (isAcActive (requestedTID)) {
			setTID (packet, requestedTID);
			//m_dcfQueues[getAC (packet)]->enqueue (packet);
			//m_dcfs[getAC (packet)]->requestAccess ();
		} else {
			// XXX ?
		}
	} else {
		if (isStreamActive (requestedTID)) {
			setTID (packet, requestedTID);
			//m_streamQueues[requestedTID]->enqueue (packet);
		} else {
			// XXX ?
		}
	}
}

void 
MacHighQStation::gotCFPoll (Packet *packet)
{
	if (getTID (packet) > 8) {
		m_hcca->streamAccessGranted (getTID (packet), getTxopLimit (packet));
	} else {
		m_hcca->acAccessGranted (getAC (packet), getTxopLimit (packet));
	}
}

void 
MacHighQStation::flush (void)
{
	// XXX
}
