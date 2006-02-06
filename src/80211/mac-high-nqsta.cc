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

#include "mac-high-nqsta.h"
#include "dcf.h"
#include "mac-dcf-parameters.h"
#include "mac-queue-80211e.h"
#include "dca-txop.h"
#include "net-interface-80211.h"


MacHighNqsta::MacHighNqsta (int apAddress)
	: MacHighSta (apAddress)
{
	m_dcfParameters = new MacDcfParameters ();
	m_dcf = new Dcf (m_dcfParameters);
	m_dcfQueue = new MacQueue80211e ();
}

MacHighNqsta::~MacHighNqsta ()
{}

void
MacHighNqsta::setInterface (NetInterface80211 *interface)
{
	MacHighSta::notifySetNetInterface (interface);
	m_dcfParameters->setParameters (interface->parameters ());
	m_dcf->setInterface (interface);
	m_dcfQueue->setParameters (interface->parameters ());
	DcaTxop *dcaTxop = new DcaTxop (m_dcf, m_dcfQueue);
	dcaTxop->setInterface (interface);
}

void
MacHighNqsta::gotCFPoll (Packet *packet)
{
	Packet::free (packet);
}
void
MacHighNqsta::enqueueToLow (Packet *packet)
{
	m_dcfQueue->enqueue (packet);
	m_dcf->requestAccess ();
}

void
MacHighNqsta::flush (void)
{
	m_dcfQueue->flush ();
}

void 
MacHighNqsta::gotBeacon (Packet *packet)
{}
void 
MacHighNqsta::gotAssociated (Packet *packet)
{}
void 
MacHighNqsta::gotReAssociated (Packet *packet)
{}
void 
MacHighNqsta::gotAddTsResponse (Packet *packet)
{}
void 
MacHighNqsta::gotDelTsResponse (Packet *packet)
{}

void 
MacHighNqsta::addTsRequest (TSpecRequest *request)
{
	/* really, this should not happen.*/
	assert (false);
}
void 
MacHighNqsta::delTsRequest (TSpecRequest *request)
{
	assert (false);
}
