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
void 
ChunkPiece::set_original (Packet *original, uint32_t size)
{
	original->ref ();
	m_original = original;
	m_size = size;
}
	

uint32_t 
ChunkPiece::get_size (void) const
{
	return m_size;
}
Chunk *
ChunkPiece::copy (void)
{
	ChunkPiece *copy = new ChunkPiece ();
	m_original->ref ();
	copy->m_original = m_original;
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
ChunkPiece::print (std::ostream *os) const
{
	*os << "(piece)"
	    << " size: " << m_size << " ";
	m_original->print (os);
}
