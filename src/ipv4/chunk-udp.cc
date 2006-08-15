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

#include "chunk-udp.h"
#include "utils.h"

namespace yans {

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
ChunkUdp::ChunkUdp ()
	: m_source_port (0xfffd),
	  m_destination_port (0xfffd),
	  m_payload_size (0xfffd),
	  m_initial_checksum (0)
{}
ChunkUdp::~ChunkUdp ()
{
	m_source_port = 0xfffe;
	m_destination_port = 0xfffe;
	m_payload_size = 0xfffe;
}

void 
ChunkUdp::set_destination (uint16_t port)
{
	m_destination_port = port;
}
void 
ChunkUdp::set_source (uint16_t port)
{
	m_source_port = port;
}
uint16_t 
ChunkUdp::get_source (void) const
{
	return m_source_port;
}
uint16_t 
ChunkUdp::get_destination (void) const
{
	return m_destination_port;
}
void 
ChunkUdp::set_payload_size (uint16_t size)
{
	m_payload_size = size;
}
uint32_t 
ChunkUdp::get_size (void) const
{
	return 8;
}

void 
ChunkUdp::initialize_checksum (Ipv4Address source, 
			       Ipv4Address destination,
			       uint8_t protocol)
{
	uint8_t buf[12];
	source.serialize (buf);
	destination.serialize (buf+4);
	buf[8] = 0;
	buf[9] = protocol;
	uint16_t udp_length = m_payload_size + get_size ();
	buf[10] = udp_length >> 8;
	buf[11] = udp_length & 0xff;

	m_initial_checksum = utils_checksum_calculate (0, buf, 12);
}



void 
ChunkUdp::print (std::ostream *os) const
{
	*os << "(udp)"
	    << ", port source=" << m_source_port
	    << ", port destination=" << m_destination_port
	    << ", length=" << m_payload_size;
}

void 
ChunkUdp::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	i.write_hton_u16 (m_source_port);
	i.write_hton_u16 (m_destination_port);
	i.write_hton_u16 (m_payload_size + get_size ());
	i.write_hton_u16 (0);

	i = buffer->begin ();
	uint16_t checksum = utils_checksum_calculate (m_initial_checksum, 
						      i.peek_data (), 
						      get_size () + m_payload_size);
	checksum = utils_checksum_complete (checksum);
	i.next (6);
	i.write_u16 (checksum);
}
void 
ChunkUdp::peek_from (Buffer const *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	m_source_port = i.read_ntoh_u16 ();
	m_destination_port = i.read_ntoh_u16 ();
	m_payload_size = i.read_ntoh_u16 () - get_size ();
	// XXX verify checksum.
}
void 
ChunkUdp::remove_from (Buffer *buffer)
{
	buffer->remove_at_start (get_size ());
}



}; // namespace yans
