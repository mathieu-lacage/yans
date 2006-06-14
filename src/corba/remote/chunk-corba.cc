/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "chunk-corba.h"
#include "yans/chunk.h"
#include "yans/buffer.h"
#include <cassert>


ChunkCorba::ChunkCorba (const ::Remote::Buffer& buffer)
	: m_buffer (buffer)
{}
ChunkCorba::~ChunkCorba ()
{}

void 
ChunkCorba::add_to (yans::Buffer *buffer) const
{
	buffer->add_at_start (m_buffer.length ());
	yans::Buffer::Iterator start = buffer->begin ();
	unsigned char const *data = m_buffer.get_buffer ();
	start.write (data, m_buffer.length ());
}
void 
ChunkCorba::remove_from (yans::Buffer *buffer)
{
	assert (false);
}
void 
ChunkCorba::print (std::ostream *os) const
{}
