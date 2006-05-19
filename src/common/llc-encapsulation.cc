/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "llc-encapsulation.h"
#include "chunk-llc.h"
#include "packet.h"
#include "chunk.h"
#include "mac-network-interface.h"
#include <cassert>

namespace yans {

void 
LlcEncapsulation::set_ipv4_callback (RxCallback callback)
{
	m_ipv4_callback = callback;
}
void 
LlcEncapsulation::set_arp_callback (RxCallback callback)
{
	m_arp_callback = callback;
}
void 
LlcEncapsulation::set_mac_interface (MacNetworkInterface *interface)
{
	m_interface = interface;
	interface->set_rx_callback (make_callback (&LlcEncapsulation::receive, this));
}
void 
LlcEncapsulation::send_ipv4 (Packet *packet, MacAddress to)
{
	ChunkLlc llc;
	llc.set_type (ChunkLlc::TYPE_IPV4);
	packet->add (&llc);
	m_interface->send (packet, to);
}
void 
LlcEncapsulation::send_arp (Packet *packet, MacAddress to)
{
	ChunkLlc llc;
	llc.set_type (ChunkLlc::TYPE_ARP);
	packet->add (&llc);
	m_interface->send (packet, to);
}

void
LlcEncapsulation::receive (Packet *packet, MacNetworkInterface *interface)
{
	assert (interface == m_interface);
	ChunkLlc llc;
	packet->remove (&llc);
	switch (llc.get_type ()) {
	case ChunkLlc::TYPE_IPV4:
		m_ipv4_callback (packet);
		break;
	case ChunkLlc::TYPE_ARP:
		m_arp_callback (packet);
		break;
	default:
		assert (false);
		//NOT REACHED
		break;
	}
}

}; // namespace yans
