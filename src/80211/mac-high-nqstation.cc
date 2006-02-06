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

#include "mac-high-nqstation.h"
#include "dcf.h"
#include "mac-dcf-parameters.h"
#include "mac-queue-80211e.h"
#include "dca-txop.h"
#include "mac-container.h"


MacHighNQStation::MacHighNQStation (MacContainer *container, int apAddress)
	: MacHighStation (container, apAddress)
{
	m_dcfParameters = new MacDcfParameters (container);
	m_dcf = new Dcf (container, m_dcfParameters);
	m_dcfQueue = new MacQueue80211e (container->parameters ());
	new DcaTxop (m_dcf, m_dcfQueue, container);
}

MacHighNQStation::~MacHighNQStation ()
{}

void
MacHighNQStation::gotCFPoll (Packet *packet)
{
	Packet::free (packet);
}
void
MacHighNQStation::enqueueToLow (Packet *packet)
{
	m_dcfQueue->enqueue (packet);
	m_dcf->requestAccess ();
}

void
MacHighNQStation::flush (void)
{
	m_dcfQueue->flush ();
}

void 
MacHighNQStation::gotBeacon (Packet *packet)
{}
void 
MacHighNQStation::gotAssociated (Packet *packet)
{}
void 
MacHighNQStation::gotReAssociated (Packet *packet)
{}
void 
MacHighNQStation::gotAddTsResponse (Packet *packet)
{}
void 
MacHighNQStation::gotDelTsResponse (Packet *packet)
{}

void 
MacHighNQStation::addTsRequest (TSpecRequest *request)
{
	/* really, this should not happen.*/
	assert (false);
}
void 
MacHighNQStation::delTsRequest (TSpecRequest *request)
{
	assert (false);
}
