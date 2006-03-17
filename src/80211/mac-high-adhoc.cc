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

#include "mac-high-adhoc.h"
#include "mac-low.h"
#include "mac-parameters.h"
#include "dcf.h"
#include "mac-dcf-parameters.h"
#include "mac-rx-middle.h"
#include "mac-queue-80211e.h"
#include "net-interface-80211.h"
#include "dca-txop.h"
#include "mac-traces.h"
#include "ll-arp.h"

#define nopeADHOC_TRACE 1

#ifdef ADHOC_TRACE
# define TRACE(format, ...) \
  printf ("HIGH ADHOC %d %f " format "\n", m_interface->getMacAddress (), now (), ## __VA_ARGS__);
#else /* DCF_TRACE */
# define TRACE(format, ...)
#endif /* DCF_TRACE */



MacHighAdhoc::MacHighAdhoc ()
	: MacHigh ()
{
}
MacHighAdhoc::~MacHighAdhoc ()
{}

void
MacHighAdhoc::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
	MacDcfParameters *parameters = new MacDcfParameters ();
	parameters->setParameters (interface->parameters ());
	m_dcf = new Dcf (parameters);
	m_dcf->setInterface (interface);
	m_queue = new MacQueue80211e ();
	m_queue->setParameters (interface->parameters ());
	DcaTxop *dcaTxop = new DcaTxop (m_dcf, m_queue);
	dcaTxop->setInterface (interface);
}

void 
MacHighAdhoc::enqueueFromLL (Packet *packet, int macDestination)
{
	TRACE ("enqueue %d to %d (%d)", getSize (packet), 
	       macDestination, m_queue->size ());
	setSource (packet, m_interface->getMacAddress ());
	setType (packet, MAC_80211_DATA);
	setFinalDestination (packet, macDestination);
	setDestination (packet, getFinalDestination (packet));
	m_queue->enqueue (packet);
	m_dcf->requestAccess ();
}
void 
MacHighAdhoc::addTsRequest (TSpecRequest *request)
{
	assert (false);
}
void 
MacHighAdhoc::delTsRequest (TSpecRequest *request)
{
	assert (false);
}

void 
MacHighAdhoc::notifyAckReceivedFor (Packet *packet)
{}

void 
MacHighAdhoc::receiveFromMacLow (Packet *packet)
{
	TRACE ("received %d from %d", getSize (packet), getSource (packet));
	m_interface->ll ()->sendUp (packet);
}

double
MacHighAdhoc::now (void)
{
	return Scheduler::instance ().clock ();
}

