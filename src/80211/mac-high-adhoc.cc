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
#include "dcf.h"
#include "mac-queue-80211e.h"
#include "dca-txop.h"
#include "network-interface-80211.h"

#define nopeADHOC_TRACE 1

#ifdef ADHOC_TRACE
# define TRACE(x) \
  printf ("HIGH ADHOC"<<x<<std::endl);
#else /* ADHOC_TRACE */
# define TRACE(x)
#endif /* ADHOC_TRACE */

namespace yans {

MacHighAdhoc::MacHighAdhoc ()
{}
MacHighAdhoc::~MacHighAdhoc ()
{}

void
MacHighAdhoc::set_interface (NetworkInterface80211 *interface)
{
	m_interface = interface;
}
void
MacHighAdhoc::set_queue (MacQueue80211e *queue, Dcf *dcf)
{
	m_queue = queue;
	m_dcf = dcf;
}

void 
MacHighAdhoc::enqueue_from_ll (Packet *packet, MacAddress to)
{
	TRACE ("enqueue size="<<packet->get_size ()<<", to="<<to<<
	       ", queue_size="<<m_queue->size ());
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	hdr.set_addr1 (to);
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (m_interface->get_bssid ());
	hdr.set_ds_not_from ();
	hdr.set_ds_not_to ();
	m_queue->enqueue (packet, hdr);
	m_dcf->request_access ();
}

void 
MacHighAdhoc::notify_ack_received_for (ChunkMac80211Hdr const *hdr)
{}

void 
MacHighAdhoc::receive (Packet *packet, ChunkMac80211Hdr const *hdr)
{
	TRACE ("received size="<<packet->get_size ()<<", from="<<hdr->get_addr2 ());
//XXX
}

}; // namespace yans
