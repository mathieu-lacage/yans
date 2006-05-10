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

#include "chunk.h"
#include "chunk-ipv4.h"
#include "utils.h"
#include "buffer.h"
#include <cassert>

#define TRACE_CHUNK_IPV4 1

#ifdef TRACE_CHUNK_IPV4
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "CHUNK IPV4 TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_CHUNK_IPV4 */
# define TRACE(format,...)
#endif /* TRACE_CHUNK_IPV4 */

namespace yans {

ChunkIpv4::ChunkIpv4 ()
	: m_payload_size (0),
	  m_identification (0),
	  m_tos (0),
	  m_ttl (0),
	  m_protocol (0),
	  m_flags (0),
	  m_fragment_offset (0)
{}
ChunkIpv4::~ChunkIpv4 ()
{}

void 
ChunkIpv4::set_payload_size (uint16_t size)
{
	m_payload_size = size;
}
uint16_t 
ChunkIpv4::get_payload_size (void) const
{
	return m_payload_size;
}

uint16_t 
ChunkIpv4::get_identification (void) const
{
	return m_identification;
}
void 
ChunkIpv4::set_identification (uint16_t identification)
{
	m_identification = identification;
}



void 
ChunkIpv4::set_tos (uint8_t tos)
{
	m_tos = tos;
}
uint8_t 
ChunkIpv4::get_tos (void) const
{
	return m_tos;
}
void 
ChunkIpv4::set_more_fragments (void)
{
	m_flags |= MORE_FRAGMENTS;
}
void
ChunkIpv4::set_last_fragment (void)
{
	m_flags &= ~MORE_FRAGMENTS;
}
bool 
ChunkIpv4::is_last_fragment (void) const
{
	return !(m_flags & MORE_FRAGMENTS);
}

void 
ChunkIpv4::set_dont_fragment (void)
{
	m_flags |= DONT_FRAGMENT;
}
void 
ChunkIpv4::set_may_fragment (void)
{
	m_flags &= ~DONT_FRAGMENT;
}
bool 
ChunkIpv4::is_dont_fragment (void) const
{
	return (m_flags & DONT_FRAGMENT);
}

void 
ChunkIpv4::set_fragment_offset (uint16_t offset)
{
	assert (!(offset & (~0x3fff)));
	m_fragment_offset = offset;
}
uint16_t 
ChunkIpv4::get_fragment_offset (void) const
{
	assert (!(m_fragment_offset & (~0x3fff)));
	return m_fragment_offset;
}

void 
ChunkIpv4::set_ttl (uint8_t ttl)
{
	m_ttl = ttl;
}
uint8_t 
ChunkIpv4::get_ttl (void) const
{
	return m_ttl;
}
	
uint8_t 
ChunkIpv4::get_protocol (void) const
{
	return m_protocol;
}
void 
ChunkIpv4::set_protocol (uint8_t protocol)
{
	m_protocol = protocol;
}

void 
ChunkIpv4::set_source (Ipv4Address source)
{
	m_source = source;
}
Ipv4Address
ChunkIpv4::get_source (void) const
{
	return m_source;
}

void 
ChunkIpv4::set_destination (Ipv4Address dst)
{
	m_destination = dst;
}
Ipv4Address
ChunkIpv4::get_destination (void) const
{
	return m_destination;
}


bool
ChunkIpv4::is_checksum_ok (void) const
{
#if DISABLE_IPV4_CHECKSUM
	return true;
#else
	return m_good_checksum;
#else
}


void 
ChunkIpv4::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	buffer->seek (0);
	
	//TRACE ("init ipv4 current="<<buffer->get_current ());
	uint8_t ver_ihl = (4 << 4) | (5);
	buffer->write_u8 (ver_ihl);
	buffer->write_u8 (m_tos);
	buffer->write_hton_u16 (m_payload_size + 5*4);
	buffer->write_hton_u16 (m_identification);
	uint32_t fragment_offset = m_fragment_offset / 8;
	uint8_t flags_frag = (fragment_offset >> 8) & 0x1f;
	if (m_flags & DONT_FRAGMENT) {
		flags_frag |= (1<<6);
	}
	if (m_flags & MORE_FRAGMENTS) {
		flags_frag |= (1<<5);
	}
	buffer->write_u8 (flags_frag);
	uint8_t frag = fragment_offset & 0xff;
	buffer->write_u8 (frag);
	buffer->write_u8 (m_ttl);
	buffer->write_u8 (m_protocol);
	buffer->write_hton_u16 (0);
	m_source.serialize (buffer);
	m_destination.serialize (buffer);

	uint8_t *data = buffer->peek_data ();
	//TRACE ("fini ipv4 current="<<state->get_current ());
	uint16_t checksum = utils_checksum_calculate (0, data, get_size ());
	checksum = utils_checksum_complete (checksum);
	//TRACE ("checksum=" <<checksum);
	buffer->seek (0+10);
	buffer->write_u16 (checksum);
}
void 
ChunkIpv4::remove_from (Buffer *buffer)
{
	buffer->seek (0);
	uint8_t ver_ihl = buffer->read_u8 ();
	uint8_t ihl = ver_ihl & 0x0f; 
	uint16_t header_size = ihl * 4;
	assert ((ver_ihl >> 4) == 4);
	m_tos = buffer->read_u8 ();
	uint16_t size = buffer->read_ntoh_u16 ();
	m_payload_size = size - header_size;
	m_identification = buffer->read_ntoh_u16 ();
	uint8_t flags = buffer->read_u8 ();
	m_flags = 0;
	if (flags & (1<<6)) {
		m_flags |= DONT_FRAGMENT;
	}
	if (flags & (1<<5)) {
		m_flags |= MORE_FRAGMENTS;
	}
	buffer->skip (-1);
	m_fragment_offset = buffer->read_ntoh_u16 ();
	m_fragment_offset *= 8;
	m_ttl = buffer->read_u8 ();
	m_protocol = buffer->read_u8 ();
	buffer->skip (2); // checksum.
	m_source.deserialize (buffer);
	m_destination.deserialize (buffer);

	uint8_t *data = buffer->peek_data ();
	//TRACE ("fini ipv4 current="<<state->get_current ());
	uint16_t local_checksum = utils_checksum_calculate (0, data, header_size);
	if (local_checksum == 0xffff) {
		m_good_checksum = true;
	} else {
		m_good_checksum = false;
	}

	buffer->remove_at_start (header_size);
}
void 
ChunkIpv4::print (std::ostream *os) const
{
	// ipv4, right ?
	*os << "(ipv4)"
	    << " tos=" << (uint32_t)m_tos
	    << ", payload length=" << utils_ntoh_16 (m_payload_size)
	    << ", id=" << m_identification
	    << ", " << (is_last_fragment ()?"last":"more")
	    << ", " << (is_dont_fragment ()?"dont":"may")
	    << ", frag offset=" << m_fragment_offset
	    << ", ttl=" << m_ttl
	    << ", protocol=" << m_protocol
	    << ", source=" << m_source
	    << ", destination=" << m_destination;
}

uint32_t 
ChunkIpv4::get_size (void) const
{
	return 5 * 4;
}

}; // namespace yans
