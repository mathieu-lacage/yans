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

#include "ipv4.h"
#include "transport-protocol.h"
#include "chunk-ipv4.h"
#include "packet.h"
#include "network-interface.h"
#include "ipv4-route.h"
#include "host.h"
#include "tag-ipv4.h"
#include "host-tracer.h"
#include "chunk-icmp.h"
#include "chunk-piece.h"
#include "defrag-state.h"


#define TRACE_IPV4 1

#ifdef TRACE_IPV4
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "IPV4 TRACE " << Simulator::instance ()->now_s () << " " << x << std::endl;
#else /* TRACE_IPV4 */
# define TRACE(format,...)
#endif /* TRACE_IPV4 */


class IcmpTransportProtocol : public TransportProtocol {
public:
	IcmpTransportProtocol ();
	virtual ~IcmpTransportProtocol ();

	virtual uint8_t get_protocol (void);
	virtual void receive (Packet *packet);
};

IcmpTransportProtocol::IcmpTransportProtocol ()
{}
IcmpTransportProtocol::~IcmpTransportProtocol ()
{}
uint8_t 
IcmpTransportProtocol::get_protocol (void)
{
	return 1;
}
void 
IcmpTransportProtocol::receive (Packet *packet)
{

}



Ipv4::Ipv4 ()
{
	m_icmp = new IcmpTransportProtocol ();
	register_transport_protocol (m_icmp);
	/* this is recommended by rfc 1700 */
	m_default_ttl = 64;
	m_defrag_states = new DefragStates ();
}
Ipv4::~Ipv4 ()
{
	delete m_icmp;
	delete m_defrag_states;
}

void 
Ipv4::set_host (Host *host)
{
	m_host = host;
}

Ipv4Route *
Ipv4::get_route (void)
{
	return m_host->get_routing_table ();
}

void 
Ipv4::set_protocol (uint8_t protocol)
{
	m_send_protocol = protocol;
}

void 
Ipv4::send (Packet *packet)
{
	ChunkIpv4 *ip_header;
	ip_header = new ChunkIpv4 ();
	ip_header->set_payload_size (packet->get_size ());

	TagOutIpv4 *tag = static_cast <TagOutIpv4 *> (packet->remove_tag (TagOutIpv4::get_tag ()));
	ip_header->set_source (tag->get_saddress ());
	ip_header->set_destination (tag->get_daddress ());
	ip_header->set_protocol (m_send_protocol);
	ip_header->set_payload_size (packet->get_size ());
	ip_header->set_ttl (m_default_ttl);

	Route const*route = tag->get_route ();
	assert (route != 0);
	packet->add_header (ip_header);

	send_out (packet, route);

	delete tag;
}

void 
Ipv4::register_transport_protocol (TransportProtocol *protocol)
{
	assert (lookup_protocol (protocol->get_protocol ()) == 0);
	m_protocols.push_back (std::make_pair (protocol->get_protocol (), protocol));
}

TransportProtocol *
Ipv4::lookup_protocol (uint8_t protocol)
{
	for (ProtocolsI i = m_protocols.begin (); i != m_protocols.end (); i++) {
		if ((*i).first == protocol) {
			return (*i).second;
		}
	}
	return 0;
}

bool
Ipv4::send_out (Packet *packet, Route const *route)
{
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (packet->peek_header ());
	ip->set_identification (m_identification);
	ip->update_checksum ();
	NetworkInterface *out_interface = route->get_interface ();
	if (packet->get_size () > out_interface->get_mtu ()) {
		if (ip->is_dont_fragment ()) {
			return false;
		}
		//XXX
		return true;
	} else {
		m_host->get_tracer ()->trace_tx_ipv4 (packet);
		if (route->is_gateway ()) {
			out_interface->send (packet, route->get_gateway ());
		} else {
			out_interface->send (packet, ip->get_destination ());
		}
		return true;
	}
}

void
Ipv4::send_icmp_time_exceeded_ttl (Packet *original, NetworkInterface *interface)
{
	Packet *packet = new Packet ();

	ChunkIpv4 *ip_header = static_cast <ChunkIpv4 *> (original->remove_header ());

	ChunkPiece *payload_piece = new ChunkPiece ();
	payload_piece->set_original (original, 8);
	packet->add_header (payload_piece);

	packet->add_header (ip_header);
	
	ChunkIcmp *icmp = new ChunkIcmp ();
	icmp->set_time_exceeded ();
	icmp->set_code (0);
	packet->add_header (icmp);

	ChunkIpv4 *ip_real = new ChunkIpv4 ();
	ip_real->set_destination (ip_header->get_source ());
	ip_real->set_source (interface->get_ipv4_address ());
	ip_real->set_payload_size (packet->get_size ());
	ip_real->set_protocol (m_icmp->get_protocol ());
	ip_real->set_ttl (m_default_ttl);
	packet->add_header (ip_real);

	Route *route = m_host->get_routing_table ()->lookup (ip_real->get_destination ());
	if (route == 0) {
		TRACE ("cannot send back icmp message to " << ip_real->get_destination ());
		return;
	}
	TRACE ("send back icmp ttl exceeded to " << ip_real->get_destination ());
	send_out (packet, route);
}

bool
Ipv4::forwarding (Packet *packet, NetworkInterface *interface)
{
	ChunkIpv4 *ip_header = static_cast <ChunkIpv4 *> (packet->peek_header ());
	NetworkInterfaces const * interfaces = m_host->get_interfaces ();
	for (NetworkInterfacesCI i = interfaces->begin ();
	     i != interfaces->end (); i++) {
		if ((*i)->get_ipv4_address ().is_equal (ip_header->get_destination ())) {
			TRACE ("for me 1");
			return false;
		}
	}
	if (ip_header->get_destination ().is_equal (interface->get_ipv4_broadcast ())) {
		TRACE ("for me 2");
		return false;
	}
	if (ip_header->get_destination ().is_equal (Ipv4Address::get_broadcast ())) {
		TRACE ("for me 3");
		return false;
	}
	if (ip_header->get_destination ().is_equal (Ipv4Address::get_any ())) {
		TRACE ("for me 4");
		return false;
	}
	if (ip_header->get_ttl () == 1) {
		send_icmp_time_exceeded_ttl (packet, interface);
		TRACE ("not for me -- ttl expired. drop.");
		return true;
	}
	ip_header->set_ttl (ip_header->get_ttl () - 1);
	Route *route = m_host->get_routing_table ()->lookup (ip_header->get_destination ());
	if (route == 0) {
		TRACE ("not for me -- forwarding but no route to host. drop.");
		return true;
	}
	TRACE ("not for me -- forwarding.");
	send_out (packet, route);
	return true;
}

Packet *
Ipv4::re_assemble (Packet *fragment)
{
	DefragState *state = m_defrag_states->lookup (fragment);
	if (state == 0) {
		state = new DefragState ();
		state->add (fragment);
		m_defrag_states->add (state);
		return 0;
	}
	state->add (fragment);
	if (state->is_complete ()) {
		Packet *completed = state->get_complete ();
		m_defrag_states->remove (state);
		delete state;
		return completed;
	}
	return 0;
}

void 
Ipv4::receive (Packet *packet, NetworkInterface *interface)
{
	m_host->get_tracer ()->trace_rx_ipv4 (packet);
	ChunkIpv4 *ip_header = static_cast <ChunkIpv4 *> (packet->peek_header ());

	if (!ip_header->is_checksum_ok ()) {
		TRACE ("checksum not ok");
		return;
	}
	if (forwarding (packet, interface)) {
		return;
	}
	if (!ip_header->is_last_fragment () ||
	    ip_header->get_fragment_offset () != 0) {
		Packet *new_packet = re_assemble (packet);
		if (new_packet == 0) {
			return;
		}
		packet = new_packet;
		ip_header = static_cast <ChunkIpv4 *> (packet->peek_header ());
	}


	/* receive the packet. */
	packet->remove_header ();
	TagInIpv4 *tag = new TagInIpv4 (interface);
	packet->add_tag (TagInIpv4::get_tag (), tag);

	tag->set_daddress (ip_header->get_destination ());
	TransportProtocol *protocol = lookup_protocol (ip_header->get_protocol ());
	delete ip_header;
	if (protocol != 0) {
		protocol->receive (packet);
	}
}
