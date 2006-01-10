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

namespace yans {

ChunkMacLlcSnap::ChunkMacLlcSnap ()
{}

ChunkMacLlcSnap::~ChunkMacLlcSnap ()
{}

void 
ChunkMacLlcSnap::set_source (MacAddress source)
{
	m_source = source;
}
void 
ChunkMacLlcSnap::set_destination (MacAddress dest)
{
	m_destination = dest;
}
void 
ChunkMacLlcSnap::set_length (uint16_t length)
{
	assert (length <= 0x05dc);
	m_length = length;
}
uint16_t 
ChunkMacLlcSnap::get_length (void)
{
	assert (m_length <= 0x05dc);
	return m_length;
}
MacAddress 
ChunkMacLlcSnap::get_source (void)
{
	return m_source;
}
MacAddress 
ChunkMacLlcSnap::get_destination (void)
{
	return m_destination;
}
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
	return 6 + 6 + 2 + 1 + 1 + 1 + 3 + 2;
}
void 
ChunkMacLlcSnap::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	buffer->seek (0);
	m_source.serialize (buffer);
	m_destination.serialize (buffer);
	assert (m_length <= 0x05dc);
	/* ieee 802.3 says length is msb. */
	buffer->write_hton_u16 (m_length + 8);
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
	buffer->seek (0);
	m_source.deserialize (buffer);
	m_destination.deserialize (buffer);
	m_length = buffer->read_ntoh_u16 () - 8;
	buffer->skip (6);
	m_ether_type = buffer->read_ntoh_u16 ();
	buffer->remove_at_start (get_size ());
}

void 
ChunkMacLlcSnap::print (std::ostream *os) const
{
	*os << "(mac)"
	    << " source: ";
	m_source.print (os);
	*os << " dest: ";
	m_destination.print (os);
	*os << " length: " << m_length;
	*os << " EtherType: ";
	os->setf (std::ios::hex, std::ios::basefield);
	*os << m_ether_type;
	os->setf (std::ios::dec, std::ios::basefield);
}


}; // namespace yans
