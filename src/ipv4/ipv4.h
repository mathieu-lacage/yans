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

class NetworkInterface;
class Packet;
class Ipv4Route;
class Host;
class IcmpTransportProtocol;
class Route;
class DefragStates;
class ChunkIpv4;

class Ipv4 {
public:
	typedef Callback<void (Packet *)> TransportProtocolCallback;

	Ipv4 ();
	~Ipv4 ();

	void set_host (Host *host);

	void set_protocol (uint8_t protocol);
	void send (Packet *packet);

	/* invoked from higher-layers. */
	void register_transport_protocol (TransportProtocolCallback *callback, uint8_t protocol);
	/* invoked from lower-layers. */
	void receive (Packet *packet, NetworkInterface *from);

private:
	bool forwarding (Packet *packet, NetworkInterface *from);
	Ipv4Route *get_route (void);
	TransportProtocolCallback *lookup_protocol (uint8_t id);
	void send_icmp_time_exceeded_ttl (Packet *original, NetworkInterface *interface);
	bool send_out (Packet *packet, Route const*route);
	void send_real_out (Packet *packet, Route const*route);
	Packet *re_assemble (Packet *fragment);
	void receive_packet (Packet *packet, ChunkIpv4 *ip, NetworkInterface *interface);
	void receive_icmp (Packet *packet);

	static const uint8_t ICMP_PROTOCOL;

	Host *m_host;
	typedef std::vector<std::pair<uint8_t, TransportProtocolCallback *> > Protocols;
	typedef std::vector<std::pair<uint8_t, TransportProtocolCallback *> >::iterator ProtocolsI;
	Protocols m_protocols;
	Ipv4Address m_send_destination;
	uint8_t m_send_protocol;
	uint8_t m_default_ttl;
	TransportProtocolCallback *m_icmp_callback;
	uint16_t m_identification;
	DefragStates *m_defrag_states;
};

#endif /* IPV4_H */

