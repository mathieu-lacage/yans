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

namespace yans {

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
void 
ChunkMacCrc::print (std::ostream *os) const
{
	*os << "(mac) crc";
}
void 
ChunkMacCrc::add_to (GBuffer *buffer) const
{
	uint32_t end = buffer->get_size ();
	buffer->add_at_end (get_size ());
	GBuffer::Iterator i = buffer->begin ();
	i.next (end);
	for (uint8_t j = 0; j < m_pad_size; j++) {
		i.write_u8 (j);
	}
	i.write_u32 (0);
}
void 
ChunkMacCrc::peek_from (GBuffer const *buffer)
{}
void 
ChunkMacCrc::remove_from (GBuffer *buffer)
{
	buffer->remove_at_end (get_size ());
}


}; // namespace yans
