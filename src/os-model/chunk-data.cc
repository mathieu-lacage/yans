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

#include "chunk-data.h"
#include "buffer.h"
#include <string.h>

ChunkData::ChunkData (uint8_t const *buffer, uint32_t len)
	: m_data (new uint8_t [len]),
	  m_len (len)
{
	memcpy (m_data, buffer, len);
}
ChunkData::~ChunkData ()
{
	delete [] m_data;
	m_data = (uint8_t *)0xdeadbeaf;
	m_len = 0x33;
}

uint32_t 
ChunkData::get_size (void) const
{
	return m_len;
}
Chunk *
ChunkData::copy (void)
{
	ChunkData *other = new ChunkData (m_data, m_len);
	return other;
}
void 
ChunkData::serialize (WriteBuffer *buffer)
{
	buffer->write (m_data, m_len);
}
void 
ChunkData::deserialize (ReadBuffer *buffer)
{
	buffer->read (m_data, m_len);
}
void 
ChunkData::print (std::ostream *os) const
{
	*os << "(data)";
	*os << " len: " << m_len;
}
