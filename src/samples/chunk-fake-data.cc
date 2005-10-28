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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#include "chunk-fake-data.h"
#include "buffer.h"
#include <string.h>

ChunkFakeData::ChunkFakeData (uint32_t len)
	: m_len (len)
{}
ChunkFakeData::~ChunkFakeData ()
{
	m_len = 0x33333333;
}

uint32_t 
ChunkFakeData::get_size (void)
{
	return m_len;
}
Chunk *
ChunkFakeData::copy (void)
{
	ChunkFakeData *other = new ChunkFakeData (m_len);
	return other;
}
void 
ChunkFakeData::serialize (WriteBuffer *buffer)
{
	assert (m_len > 4);
	buffer->write_hton_u32 (m_len);
	for (uint32_t i = 0; i < m_len - 4; i++) {
		buffer->write_u8 (0x55);
	}
}
void 
ChunkFakeData::deserialize (ReadBuffer *buffer)
{
	m_len = buffer->read_ntoh_u32 ();
	assert (m_len > 4);
	for (uint32_t i = 0; i < m_len - 4; i++) {
		uint8_t data = buffer->read_u8 ();
		assert (data == 0x55);
	}
}
void 
ChunkFakeData::print (std::ostream *os)
{
	*os << "(fake data)"
	    << " len: " << m_len;
}
