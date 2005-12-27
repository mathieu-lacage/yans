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

#include "chunk.h"

uint32_t 
ChunkSerializationState::get_current (void)
{
	return m_current;
}
uint32_t 
ChunkSerializationState::get_next (void)
{
	return m_next;
}
uint32_t 
ChunkSerializationState::get_prev (void)
{
	return m_prev;
}
Chunk *
ChunkSerializationState::get_prev_chunk (void)
{
	return m_prev_chunk;
}
Chunk *
ChunkSerializationState::get_next_chunk (void)
{
	return m_next_chunk;
}

void 
ChunkSerializationState::set (Chunk *prev_chunk, Chunk *next_chunk,
			      uint32_t prev, uint32_t next,
			      uint32_t current)
{
	m_prev_chunk = prev_chunk;
	m_next_chunk = next_chunk;
	m_prev = prev;
	m_next = next;
	m_current = current;
}


Chunk::~Chunk ()
{}

void 
Chunk::serialize_init (Buffer *buffer) const
{}
void 
Chunk::serialize_fini (Buffer *buffer, 
		       ChunkSerializationState *state) const
{}



std::ostream& operator<< (std::ostream& os, Chunk const& chunk)
{
	chunk.print (&os);
	return os;
}
