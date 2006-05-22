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

#include "chunk-arp.h"
#include "buffer.h"
#include "chunk-utils.h"
#include <cassert>



namespace yans {

ChunkArp::~ChunkArp ()
{}

void 
ChunkArp::set_request (MacAddress source_hardware_address,
		       Ipv4Address source_protocol_address,
		       Ipv4Address destination_protocol_address)
{
	m_type = ARP_TYPE_REQUEST;
	m_mac_source = source_hardware_address;
	m_mac_dest = MacAddress::get_broadcast ();
	m_ipv4_source = source_protocol_address;
	m_ipv4_dest = destination_protocol_address;
}
void 
ChunkArp::set_reply (MacAddress source_hardware_address,
		     Ipv4Address source_protocol_address,
		     MacAddress destination_hardware_address,
		     Ipv4Address destination_protocol_address)
{
	m_type = ARP_TYPE_REPLY;
	m_mac_source = source_hardware_address;
	m_mac_dest = destination_hardware_address;
	m_ipv4_source = source_protocol_address;
	m_ipv4_dest = destination_protocol_address;
}
bool 
ChunkArp::is_request (void) const
{
	return (m_type == ARP_TYPE_REQUEST)?true:false;
}
bool 
ChunkArp::is_reply (void) const
{
	return (m_type == ARP_TYPE_REPLY)?true:false;
}
MacAddress 
ChunkArp::get_source_hardware_address (void)
{
	return m_mac_source;
}
MacAddress 
ChunkArp::get_destination_hardware_address (void)
{
	return m_mac_dest;
}
Ipv4Address 
ChunkArp::get_source_ipv4_address (void)
{
	return m_ipv4_source;
}
Ipv4Address 
ChunkArp::get_destination_ipv4_address (void)
{
	return m_ipv4_dest;
}


uint32_t 
ChunkArp::get_size (void) const
{
	/* this is the size of an ARP payload. */
	return 28;
}

void 
ChunkArp::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();

	/* ethernet */
	i.write_hton_u16 (0x0001);
	/* ipv4 */
	i.write_hton_u16 (0x0800);
	i.write_u8 (6);
	i.write_u8 (4);
	i.write_hton_u16 (m_type);
	write_to (i, m_mac_source);
	write_to (i, m_ipv4_source);
	write_to (i, m_mac_dest);
	write_to (i, m_ipv4_dest);
}
void 
ChunkArp::remove_from (Buffer *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i.next (2+2+1+1);
	m_type = i.read_ntoh_u16 ();
	read_from (i, m_mac_source);
	read_from (i, m_ipv4_source);
	read_from (i, m_mac_dest);
	read_from (i, m_ipv4_dest);
	buffer->remove_at_start (get_size ());
}
void 
ChunkArp::print (std::ostream *os) const
{
	*os << "(arp)";
	if (is_request ()) {
		*os << " source mac: " << m_mac_source
		    << " source ipv4: " << m_ipv4_source
		    << " dest ipv4: " << m_ipv4_dest;
	} else {
		assert (is_reply ());
		*os << " source mac: " << m_mac_source
		    << " source ipv4: " << m_ipv4_source
		    << " dest mac: " << m_mac_dest
		    << " dest ipv4: " <<m_ipv4_dest;
	}
}

}; // namespace yans
