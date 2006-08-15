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
#include "chunk-ipv4.h"
#include "gpacket.h"
#include "ipv4-network-interface.h"
#include "ipv4-route.h"
#include "host.h"
#include "tag-ipv4.h"
#include "chunk-icmp.h"
#include "defrag-state.h"
#include "packet-logger.h"
#include "trace-container.h"
#include <cassert>


#define noTRACE_IPV4 1

#ifdef TRACE_IPV4
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "IPV4 TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_IPV4 */
# define TRACE(format,...)
#endif /* TRACE_IPV4 */

namespace yans {

const uint8_t Ipv4::ICMP_PROTOCOL = 1;


Ipv4::Ipv4 ()
{
	m_routing_table = new Ipv4Route ();
	register_transport_protocol (make_callback (&Ipv4::receive_icmp, this), 
				     ICMP_PROTOCOL);
	/* this is recommended by rfc 1700 */
	m_default_ttl = 64;
	m_defrag_states = new DefragStates ();
	m_identification = 0;
	m_send_logger = new PacketLogger ();
	m_recv_logger = new PacketLogger ();
}
Ipv4::~Ipv4 ()
{
	delete m_defrag_states;
	m_defrag_states = (DefragStates *)0xdeadbeaf;
	m_protocols.erase (m_protocols.begin (), m_protocols.end ());
	delete m_send_logger;
	delete m_recv_logger;
	delete m_routing_table;
	for (Ipv4NetworkInterfacesI i = m_interfaces.begin ();
	     i != m_interfaces.end (); i++) {
		delete (*i);
	}
	m_interfaces.erase (m_interfaces.begin (), m_interfaces.end ());
}

void 
Ipv4::register_trace (TraceContainer *container)
{
	container->register_packet_logger ("ipv4-send", m_send_logger);
	container->register_packet_logger ("ipv4-recv", m_recv_logger);
}

Ipv4Route *
Ipv4::get_routing_table (void)
{
	return m_routing_table;
}

uint32_t 
Ipv4::add_interface (Ipv4NetworkInterface *interface, 
		     Ipv4Address address, Ipv4Mask mask)
{
	interface->set_address (address);
	interface->set_mask (mask);
	interface->set_rx_callback (make_callback (&Ipv4::receive, this));
	m_interfaces.push_back (interface);
	return m_interfaces.size () - 1;
}


void 
Ipv4::set_protocol (uint8_t protocol)
{
	m_send_protocol = protocol;
}

void 
Ipv4::send (Packet packet)
{
	ChunkIpv4 ip_header;

	TagOutIpv4AddressPair tag;
	packet.peek_tag (&tag);
	ip_header.set_source (tag.m_saddr);
	ip_header.set_destination (tag.m_daddr);
	ip_header.set_protocol (m_send_protocol);
	ip_header.set_payload_size (packet.get_size ());
	ip_header.set_ttl (m_default_ttl);
	ip_header.set_may_fragment ();
	ip_header.set_identification (m_identification);

	m_identification ++;

	Route route;
	bool found;
	found = packet.peek_tag (&route);
	assert (found);

	send_out (packet, &ip_header, &route);
}

void 
Ipv4::register_transport_protocol (TransportProtocolCallback callback, uint8_t protocol)
{
	assert (lookup_protocol (protocol).is_null ());
	m_protocols.push_back (std::make_pair (protocol, callback));
}

Ipv4::TransportProtocolCallback 
Ipv4::lookup_protocol (uint8_t protocol)
{
	for (ProtocolsI i = m_protocols.begin (); i != m_protocols.end (); i++) {
		if ((*i).first == protocol) {
			return (*i).second;
		}
	}
	return TransportProtocolCallback ();
}

void
Ipv4::send_real_out (Packet packet, ChunkIpv4 *ip, Route const *route)
{

	packet.add (ip);
	Ipv4NetworkInterface *out_interface = m_interfaces[route->get_interface ()];
	assert (packet.get_size () <= out_interface->get_mtu ());
	m_send_logger->log (packet);
	if (route->is_gateway ()) {
		out_interface->send (packet, route->get_gateway ());
	} else {
		out_interface->send (packet, ip->get_destination ());
	}
}

bool
Ipv4::send_out (Packet packet, ChunkIpv4 *ip, Route const *route)
{
	Ipv4NetworkInterface *out_interface = m_interfaces[route->get_interface ()];

	if (packet.get_size () + ip->get_size () > out_interface->get_mtu ()) {
		/* we need to fragment. */
		if (ip->is_dont_fragment ()) {
			/* we are not allowed to fragment this packet. */
			return false;
		}
		uint16_t fragment_length = (out_interface->get_mtu () - 20) & (~0x7);
		uint16_t last_fragment_length = packet.get_size () % fragment_length;
		uint16_t n_fragments = packet.get_size () / fragment_length + 1;
		uint16_t current_offset = ip->get_fragment_offset ();

		assert (n_fragments > 1);
		ChunkIpv4 ip_fragment = *ip;
		for (uint16_t i = 0; i < n_fragments - 1; i++) {
			Packet fragment = packet.create_fragment (current_offset - ip->get_fragment_offset (),
								   fragment_length);

			ip_fragment.set_more_fragments ();
			ip_fragment.set_fragment_offset (current_offset);
			ip_fragment.set_payload_size (fragment_length);

			send_real_out (fragment, &ip_fragment, route);
			current_offset += fragment_length;
		}

		/* generate the last fragment */
		Packet last_fragment = packet.create_fragment (current_offset - ip->get_fragment_offset (),
								last_fragment_length);
		ip_fragment = *ip;
		if (!ip->is_last_fragment ()) {
			/* this is the last fragment of an ipv4 fragment. */
			ip_fragment.set_more_fragments ();
		} else if (ip->get_fragment_offset () != 0) {
			/* this is the last fragment of the last fragment of an ipv4 packet.*/
			ip_fragment.set_last_fragment ();
		} else {
			ip_fragment.set_last_fragment ();
		}
		ip_fragment.set_fragment_offset (current_offset);
		ip_fragment.set_payload_size (last_fragment_length);

		send_real_out (last_fragment, &ip_fragment, route);
	} else {
		send_real_out (packet, ip, route);
	}
	return true;
}

void
Ipv4::send_icmp_time_exceeded_ttl (Packet original, ChunkIpv4 *ip, Ipv4NetworkInterface *interface)
{
	Packet packet = original;
	packet.add (ip);
	packet.remove_at_end (packet.get_size ()- (ip->get_payload_size () + 8));
	
	ChunkIcmp icmp;
	icmp.set_time_exceeded ();
	icmp.set_code (0);
	packet.add (&icmp);

	ChunkIpv4 ip_real;
	ip_real.set_destination (ip->get_source ());
	ip_real.set_source (interface->get_address ());
	ip_real.set_payload_size (packet.get_size ());
	ip_real.set_protocol (ICMP_PROTOCOL);
	ip_real.set_ttl (m_default_ttl);
	ip_real.set_identification (m_identification);

	Route *route = m_routing_table->lookup (ip_real.get_destination ());
	if (route == 0) {
		TRACE ("cannot send back icmp message to " << ip_real.get_destination ());
		return;
	}
	TRACE ("send back icmp ttl exceeded to " << ip_real.get_destination ());
	m_identification ++;

	send_out (packet, &ip_real, route);
}

bool
Ipv4::forwarding (Packet packet, ChunkIpv4 *ip_header, Ipv4NetworkInterface *interface)
{
	for (Ipv4NetworkInterfacesCI i = m_interfaces.begin ();
	     i != m_interfaces.end (); i++) {
		if ((*i)->get_address ().is_equal (ip_header->get_destination ())) {
			TRACE ("for me 1");
			return false;
		}
	}
	if (ip_header->get_destination ().is_equal (interface->get_broadcast ())) {
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
		send_icmp_time_exceeded_ttl (packet, ip_header, interface);
		TRACE ("not for me -- ttl expired. drop.");
		return true;
	}
	ip_header->set_ttl (ip_header->get_ttl () - 1);
	Route *route = m_routing_table->lookup (ip_header->get_destination ());
	if (route == 0) {
		TRACE ("not for me -- forwarding but no route to host. drop.");
		return true;
	}
	TRACE ("not for me -- forwarding.");
	send_out (packet, ip_header, route);
	return true;
}

Packet 
Ipv4::re_assemble (Packet fragment, ChunkIpv4 *ip, bool *complete)
{
	DefragState *state = m_defrag_states->lookup (ip);
	if (state == 0) {
		state = new DefragState (ip);
		state->add (fragment, ip);
		m_defrag_states->add (state);
		*complete = false;
		return Packet ();
	}
	state->add (fragment, ip);
	if (state->is_complete ()) {
		Packet completed = state->get_complete ();
		m_defrag_states->remove (state);
		delete state;
		*complete = true;
		return completed;
	}
	*complete = false;
	return Packet ();
}

void
Ipv4::receive_packet (Packet packet, ChunkIpv4 *ip, Ipv4NetworkInterface *interface)
{
	/* receive the packet. */
	TagInIpv4AddressPair tag;
	tag.m_daddr = ip->get_destination ();
	tag.m_saddr = ip->get_source ();
	packet.add_tag (&tag);
	TransportProtocolCallback protocol = lookup_protocol (ip->get_protocol ());
	if (!protocol.is_null ()) {
		protocol (packet);
	}
}

void 
Ipv4::receive (Packet packet, Ipv4NetworkInterface *interface)
{
	m_recv_logger->log (packet);
	ChunkIpv4 ip_header;
	packet.peek (&ip_header);
	packet.remove (&ip_header);

	if (!ip_header.is_checksum_ok ()) {
		TRACE ("checksum not ok. " << ip_header );
		return;
	}
	if (forwarding (packet, &ip_header, interface)) {
		return;
	}
	if (!ip_header.is_last_fragment () ||
	    ip_header.get_fragment_offset () != 0) {
		bool complete;
		Packet new_packet = re_assemble (packet, &ip_header, &complete);
		if (!complete) {
			TRACE ("reassemble fragment not finished.");
			return;
		}
		TRACE ("reassemble fragment finished.");
		receive_packet (new_packet, &ip_header, interface);
		return;
	}

	receive_packet (packet, &ip_header, interface);
}


void 
Ipv4::receive_icmp (Packet packet)
{}

}; // namespace yans
