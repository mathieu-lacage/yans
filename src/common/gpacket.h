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
#ifndef GPACKET_H
#define GPACKET_H

#include <stdint.h>
#include "gbuffer.h"
#include "chunk.h"

namespace yans {

class GPacket {
public:
	inline GPacket ();
	inline GPacket create_fragment (uint32_t start, uint32_t length) const;
	inline uint32_t get_size (void) const;
	inline void add (Chunk *chunk);
	inline void peek (Chunk *chunk) const;
	inline void remove (Chunk *chunk);
private:
	inline GPacket (GBuffer buffer);
	GBuffer m_buffer;
};

}; // namespace yans

namespace yans {

GPacket::GPacket ()
	: m_buffer () {}

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
	chunk->add (m_buffer);
}

void 
GPacket::peek (Chunk *chunk) const
{
	chunk->peek (m_buffer);
}

void 
GPacket::remove (Chunk *chunk)
{
	chunk->remove (m_buffer);
}


GPacket::GPacket (GBuffer buffer)
	: m_buffer (buffer)
{}

}; // namespace yans

#endif /* GPACKET_H */
