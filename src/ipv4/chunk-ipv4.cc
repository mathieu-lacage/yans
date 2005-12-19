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

ChunkIpv4::ChunkIpv4 ()
	: m_ver_ihl (5 | (4 << 4)),
	  m_tos (0),
	  m_total_length (20),
	  m_id (0),
	  m_fragment_offset (0),
	  m_ttl (0),
	  m_protocol (0),
	  m_checksum (0),
	  m_source (0),
	  m_destination (0),
	  m_payload_size (0)
{}
ChunkIpv4::~ChunkIpv4 ()
{}

void 
ChunkIpv4::set_payload_size (uint16_t size)
{
	m_total_length = utils_hton_16 (size + get_size ());
	m_payload_size = size;
}
uint16_t 
ChunkIpv4::get_payload_size (void) const
{
	return m_payload_size;
}

uint16_t 
ChunkIpv4::get_identification () const
{
	return utils_ntoh_16 (m_id);
}
void 
ChunkIpv4::set_identification (uint16_t identification)
{
	m_id = utils_hton_16 (identification);
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
ChunkIpv4::set_id (uint16_t id)
{
	m_id = utils_hton_16 (id);
}
uint16_t 
ChunkIpv4::get_id (void) const
{
	return utils_ntoh_16 (m_id);
}
void
ChunkIpv4::set_control_flag (uint8_t flag, uint8_t val)
{
	uint16_t fragment_offset = get_fragment_offset ();
	uint8_t flags = (utils_ntoh_16 (m_fragment_offset) >> 13) & 0x7;
	if (val) {
		flags |= (1<<flag);
	} else {
		flags &= ~(1<<flag);
	}
	uint16_t new_fragment_offset = (flags << 13)| fragment_offset;
	m_fragment_offset = utils_hton_16 (new_fragment_offset);
}
bool
ChunkIpv4::is_control_flag (uint8_t flag) const
{
	uint8_t flags = (utils_ntoh_16 (m_fragment_offset) >> 13) & 0x7;
	if (flags & (1<<flag)) {
		return true;
	} else {
		return false;
	}
}
void 
ChunkIpv4::set_more_fragments (void)
{
	set_control_flag (0, 1);
}
void
ChunkIpv4::set_last_fragment (void)
{
	set_control_flag (0, 0);
}
bool 
ChunkIpv4::is_last_fragment (void) const
{
	return !is_control_flag (0);
}

void 
ChunkIpv4::set_dont_fragment (void)
{
	set_control_flag (1, 1);
}
void 
ChunkIpv4::set_may_fragment (void)
{
	set_control_flag (1, 0);
}
bool 
ChunkIpv4::is_dont_fragment (void) const
{
	return is_control_flag (1);
}

void 
ChunkIpv4::set_fragment_offset (uint16_t offset)
{
	uint16_t flags = utils_ntoh_16 (m_fragment_offset) & 0xe000;
	uint16_t new_fragment_offset = flags | (offset >> 3);
	m_fragment_offset = utils_hton_16 (new_fragment_offset);
}
uint16_t 
ChunkIpv4::get_fragment_offset (void) const
{
	uint16_t fragment_offset = utils_ntoh_16 (m_fragment_offset) & 0x1fff;
	return (fragment_offset << 3);
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
	m_source = utils_hton_32 (source.get_host_order ());
}
Ipv4Address
ChunkIpv4::get_source (void) const
{
	return Ipv4Address (utils_ntoh_32 (m_source));
}

void 
ChunkIpv4::set_destination (Ipv4Address dst)
{
	m_destination = utils_hton_32 (dst.get_host_order ());
}
Ipv4Address
ChunkIpv4::get_destination (void) const
{
	return Ipv4Address (utils_ntoh_32 (m_destination));
}


uint32_t 
ChunkIpv4::get_size (void) const
{
	uint8_t ihl = m_ver_ihl & 0x0f;
	uint32_t size = ihl * 4;
	return size;
}

Chunk *
ChunkIpv4::copy (void) const
{
	ChunkIpv4 *ipv4 = new ChunkIpv4 (*this);
	return ipv4;
}
bool
ChunkIpv4::is_checksum_ok (void) const
{
	uint16_t checksum = calculate_checksum ((uint8_t *)&m_ver_ihl, 20);
	if (checksum == 0) {
		return true;
	} else {
		return false;
	}
}

void 
ChunkIpv4::update_checksum (void)
{
	m_checksum = 0;
	m_checksum = calculate_checksum ((uint8_t *)&m_ver_ihl, 20);
}

void 
ChunkIpv4::serialize (WriteBuffer *buffer)
{
	buffer->write_u8 (m_ver_ihl);
	buffer->write_u8 (m_tos);
	buffer->write ((uint8_t*)&m_total_length, 2);
	buffer->write ((uint8_t*)&m_id, 2);
	buffer->write ((uint8_t*)&m_fragment_offset, 2);
	buffer->write_u8 (m_ttl);
	buffer->write_u8 (m_protocol);
	buffer->write ((uint8_t*)&m_checksum, 2);
	buffer->write ((uint8_t*)&m_source, 4);
	buffer->write ((uint8_t*)&m_destination, 4);
}

void 
ChunkIpv4::deserialize (ReadBuffer *buffer)
{
	m_ver_ihl = buffer->read_u8 ();
	m_tos = buffer->read_u8 ();
	buffer->read ((uint8_t*)&m_total_length, 2);
	buffer->read ((uint8_t*)&m_id, 2);
	buffer->read ((uint8_t*)&m_fragment_offset, 2);
	m_ttl = buffer->read_u8 ();
	m_protocol = buffer->read_u8 ();
	buffer->read ((uint8_t*)&m_checksum, 2);
	buffer->read ((uint8_t*)&m_source, 4);
	buffer->read ((uint8_t*)&m_destination, 4);
}

void 
ChunkIpv4::print (std::ostream *os) const
{
	// ipv4, right ?
	*os << "(ipv4)"
	    << " ihl: " << (uint32_t)(m_ver_ihl & 0x0f)
	    << " tos: " << (uint32_t)m_tos
	    << " total length: " << utils_ntoh_16 (m_total_length)
	    << " id: " << get_identification ()
	    << " may/don't frag: " << is_control_flag (1)
	    << " last/more frag: " << is_control_flag (0)
	    << " frag offset: " << get_fragment_offset ()
	    << " ttl: " << (uint32_t)get_ttl ()
	    << " protocol: " << (uint32_t)get_protocol ()
	    << " checksum: " << m_checksum
	    << " source: " << Ipv4Address (utils_ntoh_32 (m_source))
	    << " destination: " << Ipv4Address (utils_ntoh_32 (m_destination));
}
