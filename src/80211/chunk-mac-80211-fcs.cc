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
#include "chunk-mac-80211-fcs.h"
#include "buffer.h"
#include <cassert>

namespace yans {

ChunkMac80211Fcs::ChunkMac80211Fcs ()
{}

ChunkMac80211Fcs::~ChunkMac80211Fcs ()
{}

uint32_t 
ChunkMac80211Fcs::get_size (void) const
{
	return 4;
}

void 
ChunkMac80211Fcs::add_to (Buffer *buffer) const
{
	buffer->add_at_end (4);
	Buffer::Iterator i = buffer->end ();
	i.prev (4);
	i.write_u32 (0);
}
void 
ChunkMac80211Fcs::peek_from (Buffer const*buffer)
{
	Buffer::Iterator i = buffer->end ();
	i.prev (4);
	// unused var: quiet compiler
	//uint32_t fcs = i.read_u32 ();
	//assert (fcs == 0);
}
void 
ChunkMac80211Fcs::remove_from (Buffer *buffer)
{
	buffer->remove_at_end (4);
}

void 
ChunkMac80211Fcs::print (std::ostream *os) const
{}
void 
ChunkMac80211Fcs::add_to (GBuffer buffer) const
{
	buffer.add_at_end (4);
	GBuffer::Iterator i = buffer.end ();
	i.prev (4);
	i.write_u32 (0);
}
void 
ChunkMac80211Fcs::peek_from (GBuffer const buffer)
{
	GBuffer::Iterator i = buffer.end ();
	i.prev (4);
	// unused var: quiet compiler
	//uint32_t fcs = i.read_u32 ();
	//assert (fcs == 0);
}
void 
ChunkMac80211Fcs::remove_from (GBuffer buffer)
{
	buffer.remove_at_end (4);
}


}; // namespace yans
