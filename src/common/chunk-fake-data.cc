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

#include "chunk-fake-data.h"
#include "buffer.h"
#include <string.h>

ChunkFakeData::ChunkFakeData (uint32_t len, uint8_t n)
	: m_len (len), m_n (n)
{}
ChunkFakeData::~ChunkFakeData ()
{
	m_len = 0x33333333;
}

uint32_t 
ChunkFakeData::get_size (void) const
{
	return m_len;
}
Chunk *
ChunkFakeData::copy (void) const
{
	ChunkFakeData *other = new ChunkFakeData (m_len, m_n);
	return other;
}
void 
ChunkFakeData::serialize (WriteBuffer *buffer)
{
	for (uint32_t i = 0; i < m_len; i++) {
		buffer->write_u8 (m_n & 0xff);
	}
}
void 
ChunkFakeData::deserialize (ReadBuffer *buffer)
{
	// XXX
}
void 
ChunkFakeData::print (std::ostream *os) const
{
	*os << "(fake data)"
	    << " len: " << m_len;
}
