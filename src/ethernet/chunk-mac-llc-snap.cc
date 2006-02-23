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

#include <iostream>

#include "chunk-mac-llc-snap.h"
#include "buffer.h"
#include <cassert>

#define noTRACE_CHUNK_LLC_SNAP 1

#ifdef TRACE_CHUNK_LLC_SNAP
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "CHUNK LLCSNAP TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_CHUNK_LLC_SNAP */
# define TRACE(format,...)
#endif /* TRACE_CHUNK_LLC_SNAP */


namespace yans {

ChunkMacLlcSnap::ChunkMacLlcSnap ()
{}

ChunkMacLlcSnap::~ChunkMacLlcSnap ()
{}
void 
ChunkMacLlcSnap::set_ether_type (uint16_t ether_type)
{
	m_ether_type = ether_type;
}
uint16_t 
ChunkMacLlcSnap::get_ether_type (void)
{
	return m_ether_type;
}

uint32_t 
ChunkMacLlcSnap::get_size (void) const
{
	return 1 + 1 + 1 + 3 + 2;
}
void 
ChunkMacLlcSnap::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	buffer->seek (0);
	buffer->write_u8 (0xaa);
	buffer->write_u8 (0xaa);
	buffer->write_u8 (0x03);
	buffer->write_u8 (0);
	buffer->write_u8 (0);
	buffer->write_u8 (0);
	buffer->write_hton_u16 (m_ether_type);
}
void 
ChunkMacLlcSnap::remove_from (Buffer *buffer)
{
	buffer->seek (6);
	m_ether_type = buffer->read_ntoh_u16 ();
	buffer->remove_at_start (get_size ());
}

void 
ChunkMacLlcSnap::print (std::ostream *os) const
{
	*os << "(mac)"
	    << " EtherType: ";
	os->setf (std::ios::hex, std::ios::basefield);
	*os << m_ether_type;
	os->setf (std::ios::dec, std::ios::basefield);
}


}; // namespace yans
