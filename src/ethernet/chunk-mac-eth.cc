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

#include "chunk-mac-eth.h"
#include "buffer.h"
#include <cassert>

#define noTRACE_CHUNK_ETH 1

#ifdef TRACE_CHUNK_ETH
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "CHUNK ETH TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_CHUNK_ETH */
# define TRACE(format,...)
#endif /* TRACE_CHUNK_ETH */


namespace yans {

ChunkMacEth::ChunkMacEth ()
{}

ChunkMacEth::~ChunkMacEth ()
{}

void 
ChunkMacEth::set_source (MacAddress source)
{
	m_source = source;
}
void 
ChunkMacEth::set_destination (MacAddress dest)
{
	m_destination = dest;
}
void 
ChunkMacEth::set_length (uint16_t length)
{
	assert (length <= 0x05dc);
	m_length = length;
}
uint16_t 
ChunkMacEth::get_length (void)
{
	assert (m_length <= 0x05dc);
	return m_length;
}
MacAddress 
ChunkMacEth::get_source (void)
{
	return m_source;
}
MacAddress 
ChunkMacEth::get_destination (void)
{
	return m_destination;
}

uint32_t 
ChunkMacEth::get_size (void) const
{
	return 6 + 6 + 2;
}
void 
ChunkMacEth::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	uint8_t src[6];
	uint8_t dst[6];
	m_source.peek (src);
	m_destination.peek (dst);
	i.write (src, 6);
	i.next (6);
	i.write (dst, 6);
	i.next (6);
	assert (m_length <= 0x05dc);
	/* ieee 802.3 says length is msb. */
	TRACE ("length="<<m_length);
	i.write_hton_u16 (m_length + 8);
}
void 
ChunkMacEth::remove_from (Buffer *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	uint8_t src[6];
	uint8_t dst[6];
	i.read (src, 6);
	i.next (6);
	i.read (dst, 6);
	i.next (6);
	m_source.set (src);
	m_destination.set (dst);
	m_length = i.read_ntoh_u16 () - 8;
	TRACE ("length="<<m_length);
	buffer->remove_at_start (get_size ());
}

void 
ChunkMacEth::print (std::ostream *os) const
{
	*os << "(mac)"
	    << " source: ";
	m_source.print (os);
	*os << " dest: ";
	m_destination.print (os);
	*os << " length: " << m_length;
}


}; // namespace yans
