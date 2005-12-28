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

#include "chunk-piece.h"
#include "buffer.h"
#include "packet.h"

namespace yans {

ChunkPiece::ChunkPiece ()
{
	m_original = 0;
	m_size = 0;
}
ChunkPiece::~ChunkPiece ()
{
	m_original->unref ();
	m_original = (Packet *)0xdeadbeaf;
	m_size = 0xdeadbeaf;
}

Packet *
ChunkPiece::get_original (void)
{
	m_original->ref ();
	return m_original;
}
uint32_t 
ChunkPiece::get_offset (void)
{
	return m_offset;
}
void 
ChunkPiece::set_original (Packet *original, uint32_t offset, uint32_t size)
{
	original->ref ();
	m_original = original;
	m_size = size;
	m_offset = offset;
}

void 
ChunkPiece::trim_start (uint32_t delta)
{
	if (m_size < delta) {
		m_size = 0;
		m_offset = m_original->get_size ();
		return;
	}
	assert (m_size >= delta);
	assert (m_original->get_size () >= delta);
	m_size -= delta;
	m_offset += delta;
}
void 
ChunkPiece::trim_end (uint32_t delta)
{
	if (m_size < delta) {
		m_size = 0;
		m_offset = 0;
		return;
	}
	assert (m_original->get_size () >= delta);
	assert (m_size >= delta);
	m_size -= delta;
}

	

uint32_t 
ChunkPiece::get_size (void) const
{
	return m_size;
}
Chunk *
ChunkPiece::copy (void) const
{
	ChunkPiece *copy = new ChunkPiece ();
	copy->m_original = m_original->copy ();
	copy->m_size = m_size;
	copy->m_offset = m_offset;
	return copy;
}
void 
ChunkPiece::serialize_init (Buffer *buffer) const
{
	//XXX
	Buffer *tmp = new Buffer (m_size);
	m_original->serialize (tmp);
	assert (tmp->get_current () >= m_offset);
	uint32_t offset = tmp->get_current () - m_offset;
	if (offset >= m_size) {
		buffer->write (tmp->peek_data ()+m_offset, m_size);
	} else {
		buffer->write (tmp->peek_data ()+m_offset, offset);
		for (uint32_t left = offset; left < m_size; left++) {
			buffer->write_u8 (0);
		}
	}
	delete tmp;
}
void 
ChunkPiece::serialize_fini (Buffer *buffer,
			    ChunkSerializationState *state) const
{
	
}

void 
ChunkPiece::print (std::ostream *os) const
{
	*os << "(piece)"
	    << " size: " << m_size << " "
	    << " offset: " << m_offset << " ";
	m_original->print (os);
}

}; // namespace yans
