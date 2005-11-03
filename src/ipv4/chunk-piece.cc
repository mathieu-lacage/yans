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

ChunkPiece::ChunkPiece ()
{
	m_original = 0;
	m_size = 0;
}
ChunkPiece::~ChunkPiece ()
{
	delete m_original;
	m_original = (Chunk *)0xdeadbeaf;
	m_size = 0xdeadbeaf;
}

Chunk *
ChunkPiece::peek_original (void)
{
	return m_original;
}
void 
ChunkPiece::set_original (Chunk *original, uint32_t size)
{
	m_original = original;
	m_size = size;
}
	

uint32_t 
ChunkPiece::get_size (void)
{
	return m_size;
}
Chunk *
ChunkPiece::copy (void)
{
	ChunkPiece *copy = new ChunkPiece ();
	copy->m_original = m_original->copy ();
	copy->m_size = m_size;
	return copy;
}
void 
ChunkPiece::serialize (WriteBuffer *buffer)
{
	WriteBuffer *tmp = new WriteBuffer (m_size);
	m_original->serialize (tmp);
	if (tmp->get_written_size () >= m_size) {
		buffer->write (tmp->peek_data (), m_size);
	} else {
		buffer->write (tmp->peek_data (), tmp->get_written_size ());
		for (uint32_t left = tmp->get_written_size (); left < m_size; left++) {
			buffer->write_u8 (0);
		}
	}
}
void 
ChunkPiece::deserialize (ReadBuffer *buffer)
{
	assert (false);
}
void 
ChunkPiece::print (std::ostream *os)
{
	*os << "(piece)"
	    << " size: " << m_size << " ";
	m_original->print (os);
}
