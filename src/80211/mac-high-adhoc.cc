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
#include "dca-txop.h"
#include "network-interface-80211.h"
#include "packet.h"

#define noADHOC_TRACE 1

#ifdef ADHOC_TRACE
# include <iostream>
# define TRACE(x) \
  std::cout << "HIGH ADHOC "<<x<<std::endl;
#else /* ADHOC_TRACE */
# define TRACE(x)
#endif /* ADHOC_TRACE */

namespace yans {

MacHighAdhoc::MacHighAdhoc ()
{}
MacHighAdhoc::~MacHighAdhoc ()
{
	delete m_callback;
}

void
MacHighAdhoc::set_interface (NetworkInterface80211 *interface)
{
	m_interface = interface;

}
void 
MacHighAdhoc::set_forward_callback (ForwardCallback *callback)
{
	m_callback = callback;
}
void
MacHighAdhoc::set_dca_txop (DcaTxop *dca)
{
	m_dca = dca;
}

void 
MacHighAdhoc::enqueue (Packet *packet, MacAddress to)
{
	TRACE ("enqueue size="<<packet->get_size ()<<", to="<<to<<
	       ", queue_size="<<m_queue->get_size ());
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_DATA);
	hdr.set_addr1 (to);
	hdr.set_addr2 (m_interface->get_mac_address ());
	hdr.set_addr3 (m_interface->get_bssid ());
	hdr.set_ds_not_from ();
	hdr.set_ds_not_to ();
	m_dca->queue (packet, hdr);
}

void 
MacHighAdhoc::ack_received (ChunkMac80211Hdr const &hdr)
{}

void 
MacHighAdhoc::receive (Packet *packet, ChunkMac80211Hdr const *hdr)
{
	TRACE ("received size="<<packet->get_size ()<<", from="<<hdr->get_addr2 ());
	(*m_callback) (packet);
}

}; // namespace yans
