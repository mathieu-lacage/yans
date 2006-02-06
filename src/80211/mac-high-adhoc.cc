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
#include "mac-container.h"
#include "dca-txop.h"

MacHighAdhoc::MacHighAdhoc (MacContainer *container)
	: MacHigh (container)
{
	MacDcfParameters *parameters;
	Dcf *dcf = new Dcf (container, parameters);
	m_queue = new MacQueue80211e (container->parameters ());
	new DcaTxop (dcf, m_queue, container);
}
MacHighAdhoc::~MacHighAdhoc ()
{}

void 
MacHighAdhoc::enqueueFromLL (Packet *packet)
{
	setType (packet, MAC_80211_DATA);
	setDestination (packet, getFinalDestination (packet));
	m_queue->enqueue (packet);
}

void 
MacHighAdhoc::receiveFromPhy (Packet *packet)
{
	container ()->macLow ()->receive (packet);
}

void 
MacHighAdhoc::notifyAckReceivedFor (Packet *packet)
{}

void 
MacHighAdhoc::receiveFromMacLow (Packet *packet)
{
	container ()->forwardToLL (packet);
}
