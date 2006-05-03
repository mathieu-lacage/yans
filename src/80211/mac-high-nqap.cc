/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "mac-high-nqap.h"

namespace yans {

MacHighNqap::MacHighNqap ()
{}
MacHighNqap::~MacHighNqap ()
{}

void 
MacHighNqap::set_dca_txop (DcaTxop *dca)
{
	m_dca = dca;
}
void 
MacHighNqap::set_interface (NetworkInterface80211 *interface)
{
	m_interface = interface;
}
void 
MacHighNqap::set_forward_callback (ForwardCallback callback)
{
	m_forward = callback;
}
void 
MacHighNqap::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}
void 
MacHighNqap::queue (Packet *packet, MacAddress to)
{
	
}

void 
MacHighNqap::ack_received (ChunkMac80211Hdr const &hdr)
{}
void 
MacHighNqap::receive (Packet *packet, ChunkMac80211Hdr const *hdr)
{
	
}



}; // namespace yans
