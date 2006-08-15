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
#include "gpacket.h"
#include <cassert>

namespace yans {

GPacket::GPacket ()
	: m_buffer () {}

GPacket::GPacket (GBuffer buffer)
	: m_buffer (buffer)
{}


GPacket 
GPacket::create_fragment (uint32_t start, uint32_t length) const
{
	GBuffer tmp = m_buffer;
	tmp.remove_at_start (start);
	tmp.remove_at_end (m_buffer.get_size () - (start + length));
	return GPacket (tmp);
}

uint32_t 
GPacket::get_size (void) const
{
	return m_buffer.get_size ();
}

void 
GPacket::add (Chunk *chunk)
{
	chunk->add (&m_buffer);
}

void 
GPacket::peek (Chunk *chunk) const
{
	chunk->peek (&m_buffer);
}

void 
GPacket::remove (Chunk *chunk)
{
	chunk->remove (&m_buffer);
}


void 
GPacket::write (PacketReadWriteCallback callback) const
{
	uint8_t *data = m_buffer.begin ().peek_data ();
	uint32_t to_write = get_size ();
	callback (data, to_write);
}


void 
GPacket::add_at_end (GPacket packet)
{
	GBuffer src = packet.m_buffer;
	m_buffer.add_at_end (src.get_size ());
	GBuffer::Iterator dest_start = m_buffer.end ();
	dest_start.prev (src.get_size ());
	dest_start.write (src.begin (), src.end ());
}
void 
GPacket::add_at_end (GPacket packet, uint32_t start, uint32_t size)
{
	assert (packet.get_size () <= start + size);
	GBuffer src = packet.m_buffer;
	m_buffer.add_at_end (src.get_size ());
	GBuffer::Iterator dest_start = m_buffer.end ();
	dest_start.prev (size);
	GBuffer::Iterator src_start = src.begin ();
	src_start.next (start);
	GBuffer::Iterator src_end = src_start;
	src_end.next (size);
	dest_start.write (src_start, src_end);
}
void 
GPacket::remove_at_end (uint32_t size)
{
	m_buffer.remove_at_end (size);
}
void 
GPacket::remove_at_start (uint32_t size)
{
	m_buffer.remove_at_start (size);
}

}; // namespace yans
