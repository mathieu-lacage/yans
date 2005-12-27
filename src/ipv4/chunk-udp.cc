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
#include "buffer.h"

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
ChunkUdp::ChunkUdp ()
	: m_source_port (0xfffd),
	  m_destination_port (0xfffd),
	  m_udp_length (0xfffd)
{}
ChunkUdp::~ChunkUdp ()
{
	m_source_port = 0xfffe;
	m_destination_port = 0xffffe;
	m_udp_length = 0xfffe;
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
ChunkUdp::get_source (void)
{
	return m_source_port;
}
uint16_t 
ChunkUdp::get_destination (void)
{
	return m_destination_port;
}
void 
ChunkUdp::set_payload_size (uint16_t size)
{
	m_udp_length = size + 8;
}
uint32_t 
ChunkUdp::get_size (void) const
{
	return 8;
}
Chunk *
ChunkUdp::copy (void) const
{
	ChunkUdp *udp_chunk = new ChunkUdp ();
	*udp_chunk = *this;
	return udp_chunk;
}
void 
ChunkUdp::serialize_init (Buffer *buffer) const
{
	buffer->write_hton_u16 (m_source_port);
	buffer->write_hton_u16 (m_destination_port);
	buffer->write_hton_u16 (m_udp_length);
	buffer->write_hton_u16 (0);
}
void 
ChunkUdp::serialize_fini (Buffer *buffer,
			  ChunkSerializationState *state) const
{
	// XXX should calculate udp checksum.
}


void 
ChunkUdp::print (std::ostream *os) const
{
	*os << "(udp)"
	    << ", port source=" << m_source_port
	    << ", port destination=" << m_destination_port
	    << ", length=" << m_udp_length;
}


