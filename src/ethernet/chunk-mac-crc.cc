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

#include "chunk-mac-crc.h"
#include "buffer.h"

ChunkMacCrc::ChunkMacCrc ()
	: m_pad_size (0)
{}
ChunkMacCrc::~ChunkMacCrc ()	
{}

void
ChunkMacCrc::set_pad (uint8_t pad_size)
{
	m_pad_size = pad_size;
}

uint32_t 
ChunkMacCrc::get_size (void) const
{
	return 4 + m_pad_size;
}
Chunk *
ChunkMacCrc::copy (void)
{
	ChunkMacCrc *chunk = new ChunkMacCrc ();
	return chunk;
}
void 
ChunkMacCrc::serialize (WriteBuffer *buffer)
{
	for (uint8_t i = 0; i < m_pad_size; i++) {
		buffer->write_u8 (0x31);
	}
	buffer->write_u8 (0);
	buffer->write_u8 (0);
	buffer->write_u8 (0);
	buffer->write_u8 (0);
}
void 
ChunkMacCrc::deserialize (ReadBuffer *buffer)
{
	uint8_t crc[4];
	buffer->read (crc, 4);
	assert (crc[0] == 0);
	assert (crc[1] == 0);
	assert (crc[2] == 0);
	assert (crc[3] == 0);
}
void 
ChunkMacCrc::print (std::ostream *os) const
{
	*os << "(mac) crc";
}
