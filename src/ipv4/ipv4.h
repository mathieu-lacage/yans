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

#ifndef IPV4_H
#define IPV4_H

#include <vector>

#include "ipv4-address.h"
#include "callback.h"
#include "packet.h"

namespace yans {

class Ipv4NetworkInterface;
class Ipv4Route;
class Host;
class IcmpTransportProtocol;
class Route;
class DefragStates;
class ChunkIpv4;
class PacketLogger;
class TraceContainer;


class Ipv4 {
public:
	typedef Callback<void,PacketPtr> TransportProtocolCallback;

	Ipv4 ();
	~Ipv4 ();

	void set_protocol (uint8_t protocol);
	void send (PacketPtr packet);

	void register_trace (TraceContainer *container);

	/* invoked from higher-layers. */
	void register_transport_protocol (TransportProtocolCallback callback, uint8_t protocol);
	/* invoked from lower-layers. */
	void receive (PacketPtr packet, Ipv4NetworkInterface *from);

	uint32_t add_interface (Ipv4NetworkInterface *interface, 
				Ipv4Address address, Ipv4Mask mask);
	Ipv4Route *get_routing_table (void);
	

private:
	bool forwarding (PacketPtr packet, ChunkIpv4 *ip, Ipv4NetworkInterface *from);
	TransportProtocolCallback lookup_protocol (uint8_t id);
	void send_icmp_time_exceeded_ttl (PacketPtr original, ChunkIpv4 *ip, Ipv4NetworkInterface *interface);
	bool send_out (PacketPtr packet, ChunkIpv4 *ip, Route const*route);
	void send_real_out (PacketPtr packet, ChunkIpv4 *ip, Route const*route);
	PacketPtr re_assemble (PacketPtr fragment, ChunkIpv4 *ip);
	void receive_packet (PacketPtr packet, ChunkIpv4 *ip, Ipv4NetworkInterface *interface);
	void receive_icmp (PacketPtr packet);

	static const uint8_t ICMP_PROTOCOL;

	typedef std::vector<std::pair<uint8_t, TransportProtocolCallback > > Protocols;
	typedef std::vector<std::pair<uint8_t, TransportProtocolCallback > >::iterator ProtocolsI;
	typedef std::vector<Ipv4NetworkInterface *> Ipv4NetworkInterfaces;
	typedef std::vector<Ipv4NetworkInterface *>::iterator Ipv4NetworkInterfacesI;
	typedef std::vector<Ipv4NetworkInterface *>::const_iterator Ipv4NetworkInterfacesCI;

	Ipv4Route *m_routing_table;
	Ipv4NetworkInterfaces m_interfaces;
	Protocols m_protocols;
	Ipv4Address m_send_destination;
	uint8_t m_send_protocol;
	uint8_t m_default_ttl;
	TransportProtocolCallback m_icmp_callback;
	uint16_t m_identification;
	DefragStates *m_defrag_states;
	PacketLogger *m_send_logger;
	PacketLogger *m_recv_logger;
};

}; // namespace yans 

#endif /* IPV4_H */

