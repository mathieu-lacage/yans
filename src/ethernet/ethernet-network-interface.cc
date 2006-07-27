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

#include "ethernet-network-interface.h"
#include "cable.h"
#include "chunk-mac-crc.h"
#include "chunk-mac-eth.h"
#include "packet.h"
#include "callback.h"
#include "packet-logger.h"
#include "trace-container.h"
#include <cassert>

#define noTRACE_ETHERNET 1

#ifdef TRACE_ETHERNET
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "ETHERNET TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_ETHERNET */
# define TRACE(format,...)
#endif /* TRACE_ETHERNET */


namespace yans {

EthernetNetworkInterface::EthernetNetworkInterface (MacAddress self,
						    char const *name)
	: MacNetworkInterface (self, 1000),
	  m_cable (0),
	  m_send_logger (new PacketLogger ()),
	  m_recv_logger (new PacketLogger ())
{}

EthernetNetworkInterface::~EthernetNetworkInterface ()
{
	m_cable = (Cable *)0xdeadbeaf;
	delete m_send_logger;
	m_send_logger = 0;
	delete m_recv_logger;
	m_recv_logger = 0;
}

void 
EthernetNetworkInterface::connect_to (Cable *cable)
{
	m_cable = cable;
}
void 
EthernetNetworkInterface::recv (PacketPtr packet)
{
	TRACE ("rx init size="<<packet->get_size ());
	m_recv_logger->log (packet);
	ChunkMacEth eth;
	ChunkMacCrc trailer;
	packet->peek (&eth);
	packet->remove (&eth);
	TRACE ("rx no header size="<<packet->get_size ());
	if (eth.get_length () < packet->get_size () - 4) {
		uint32_t padding = packet->get_size () - 4 - eth.get_length ();
		TRACE ("rx read padding="<<padding);
		trailer.set_pad (padding);
	} else {
		TRACE ("rx no padding, length="<<eth.get_length ()<<", packet="<<packet->get_size ());
	}
	packet->peek (&trailer);
	packet->remove (&trailer);
	forward_up (packet);
}

void 
EthernetNetworkInterface::register_trace (TraceContainer *container)
{
	container->register_packet_logger ("ethernet-send", m_send_logger);
	container->register_packet_logger ("ethernet-recv", m_recv_logger);
}
void 
EthernetNetworkInterface::notify_up (void)
{}
void 
EthernetNetworkInterface::notify_down (void)
{}

void 
EthernetNetworkInterface::real_send (PacketPtr packet, MacAddress dest)
{
	TRACE ("tx init size="<<packet->get_size ());
	ChunkMacEth eth;
	eth.set_destination (dest);
	eth.set_source (get_mac_address ());
	eth.set_length (packet->get_size ());
	ChunkMacCrc trailer;
	if (packet->get_size () < 38) {
		trailer.set_pad (38 - packet->get_size ());
		TRACE ("tx set padding="<<38 - packet->get_size ());
	}
	packet->add (&eth);
	TRACE ("tx header size="<<packet->get_size ());
	packet->add (&trailer);
	TRACE ("tx final size="<<packet->get_size ());
	m_send_logger->log (packet);
	m_cable->send (packet, this);
}

}; // namespace yans
