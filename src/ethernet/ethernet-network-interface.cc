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

#include "ethernet-network-interface.h"
#include "cable.h"
#include "arp.h"
#include "chunk-mac-llc-snap.h"
#include "chunk-mac-crc.h"
#include "packet.h"
#include "ipv4.h"
#include "host.h"
#include "callback.tcc"
#include "packet-logger.h"
#include "trace-container.h"

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

EthernetNetworkInterface::EthernetNetworkInterface (char const *name)
	: m_name (new std::string (name)),
	  m_cable (0)
{
	m_arp = new Arp (this);
	m_arp->set_sender (make_callback (&EthernetNetworkInterface::send_data, this),
			   make_callback (&EthernetNetworkInterface::send_arp, this));
	m_mtu = 1000;
	m_send_logger = new PacketLogger ();
	m_recv_logger = new PacketLogger ();
}

EthernetNetworkInterface::~EthernetNetworkInterface ()
{
	assert (m_name != (std::string *)0xdeadbeaf);
	delete m_name;
	m_name = (std::string *)0xdeadbeaf;
	delete m_arp;
	m_arp = (Arp *)0xdeadbeaf;
	m_cable = (Cable *)0xdeadbeaf;
	delete m_send_logger;
	delete m_recv_logger;
}
void 
EthernetNetworkInterface::set_mtu (uint16_t mtu)
{
	m_mtu = mtu;
}

void 
EthernetNetworkInterface::set_host (Host *host)
{
	m_host = host;
}
void 
EthernetNetworkInterface::set_mac_address (MacAddress self)
{
	m_mac_address = self;
}
MacAddress 
EthernetNetworkInterface::get_mac_address (void)
{
	return m_mac_address;
}
std::string const *
EthernetNetworkInterface::get_name (void)
{
	return m_name;
}
uint16_t 
EthernetNetworkInterface::get_mtu (void)
{
	return m_mtu;
}
void 
EthernetNetworkInterface::set_up   (void)
{
	m_up = true;
}
void 
EthernetNetworkInterface::set_down (void)
{
	m_up = false;
}
bool 
EthernetNetworkInterface::is_down (void)
{
	return !m_up;
}
void 
EthernetNetworkInterface::set_ipv4_handler (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
EthernetNetworkInterface::set_ipv4_address (Ipv4Address address)
{
	m_ipv4_address = address;
}
void 
EthernetNetworkInterface::set_ipv4_mask    (Ipv4Mask mask)
{
	m_ipv4_mask = mask;
}
Ipv4Address 
EthernetNetworkInterface::get_ipv4_address (void)
{
	return m_ipv4_address;
}
Ipv4Mask    
EthernetNetworkInterface::get_ipv4_mask    (void)
{
	return m_ipv4_mask;
}
Ipv4Address 
EthernetNetworkInterface::get_ipv4_broadcast (void)
{
	uint32_t mask = m_ipv4_mask.get_host_order ();
	uint32_t address = m_ipv4_address.get_host_order ();
	Ipv4Address broadcast = Ipv4Address (address | (~mask));
	return broadcast;
}

void 
EthernetNetworkInterface::send (Packet *packet, Ipv4Address dest)
{
	m_arp->send_data (packet, dest);
}

void 
EthernetNetworkInterface::connect_to (Cable *cable)
{
	m_cable = cable;
}
void 
EthernetNetworkInterface::recv (Packet *packet)
{
	TRACE ("rx init size="<<packet->get_size ());
	m_recv_logger->log (packet);
	ChunkMacLlcSnap header;
	ChunkMacCrc trailer;
	packet->remove (&header);
	TRACE ("rx no header size="<<packet->get_size ());
	if (header.get_length () < packet->get_size () - 4) {
		uint32_t padding = packet->get_size () - 4 - header.get_length ();
		TRACE ("rx read padding="<<padding);
		trailer.set_pad (padding);
	} else {
		TRACE ("rx no padding, length="<<header.get_length ()<<", packet="<<packet->get_size ());
	}
	packet->remove (&trailer);
	TRACE ("rx final size="<<packet->get_size ());
	switch (header.get_ether_type ()) {
	case ETHER_TYPE_ARP:
		m_arp->recv_arp (packet);
		break;
	case ETHER_TYPE_IPV4:
		m_ipv4->receive (packet, this);
		break;
	}
}

void 
EthernetNetworkInterface::register_trace (TraceContainer *container)
{
	container->register_packet_logger ("ethernet-send", m_send_logger);
	container->register_packet_logger ("ethernet-recv", m_recv_logger);
	m_arp->register_trace (container);
}

void 
EthernetNetworkInterface::send_data (Packet *packet, MacAddress dest)
{
	TRACE ("tx init size="<<packet->get_size ());
	ChunkMacLlcSnap header;
	header.set_destination (dest);
	header.set_source (get_mac_address ());
	header.set_length (packet->get_size ());
	header.set_ether_type (ETHER_TYPE_IPV4);
	ChunkMacCrc trailer;
	if (packet->get_size () < 38) {
		trailer.set_pad (38 - packet->get_size ());
		TRACE ("tx set padding="<<38 - packet->get_size ());
	}
	packet->add (&header);
	TRACE ("tx header size="<<packet->get_size ());
	packet->add (&trailer);
	TRACE ("tx final size="<<packet->get_size ());
	m_send_logger->log (packet);
	m_cable->send (packet, this);
}

void 
EthernetNetworkInterface::send_arp (Packet *packet, MacAddress dest)
{
	TRACE ("tx init size="<<packet->get_size ());
	ChunkMacLlcSnap header;
	header.set_destination (dest);
	header.set_source (get_mac_address ());
	header.set_length (packet->get_size ());
	header.set_ether_type (ETHER_TYPE_ARP);
	ChunkMacCrc trailer;
	if (packet->get_size () < 38) {
		trailer.set_pad (38 - packet->get_size ());
		TRACE ("tx set padding="<<38 - packet->get_size ());
	}
	packet->add (&header);
	TRACE ("tx header size="<<packet->get_size ());
	packet->add (&trailer);
	TRACE ("tx final size="<<packet->get_size ());
	m_send_logger->log (packet);
	m_cable->send (packet, this);
}

}; // namespace yans
