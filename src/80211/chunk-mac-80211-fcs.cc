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
	buffer->seek (buffer->get_size ());
	buffer->skip (-4);
	buffer->write_u32 (0);
}
void 
ChunkMac80211Fcs::remove_from (Buffer *buffer)
{
	buffer->seek (buffer->get_size ());
	buffer->skip (-4);
	uint32_t fcs;
	fcs = buffer->read_u32 ();
	assert (fcs == 0);
	buffer->remove_at_end (4);
}
void 
ChunkMac80211Fcs::print (std::ostream *os) const
{}


}; // namespace yans
