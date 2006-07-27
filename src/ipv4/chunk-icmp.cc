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

#include "chunk-icmp.h"
#include "buffer.h"
#include <cassert>

namespace yans {

ChunkIcmp::ChunkIcmp ()
{}

ChunkIcmp::~ChunkIcmp ()
{}

uint8_t 
ChunkIcmp::get_type (void) const
{
	return m_type;
}
uint8_t 
ChunkIcmp::get_code (void) const
{
	return m_code;
}
void 
ChunkIcmp::set_code (uint8_t code)
{
	m_code = code;
}

void 
ChunkIcmp::set_time_exceeded (void)
{
	m_type = TIME_EXCEEDED;
}


uint32_t 
ChunkIcmp::get_size (void) const
{
	if (m_type == UNREACH  ||
	    m_type == TIME_EXCEEDED ||
	    m_type == PARAMETER_PROBLEM ||
	    m_type == INFORMATION_REQUEST ||
	    m_type == INFORMATION_REPLY ||
	    m_type == SOURCE_QUENCH  ||
	    m_type == REDIRECT) {
		return 8;
	} else if (m_type == ECHO ||
		   m_type == ECHO_REPLY) {
		return 8 + m_echo_data_size;
	} else if (m_type == TIMESTAMP ||
		   m_type == TIMESTAMP_REPLY) {
		return 20;
	} else {
		assert (false);
		return 0;
	}
}


void 
ChunkIcmp::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	i.write_u8 (m_type);
	i.write_u8 (m_code);
	i.write_hton_u16 (0);
	if (m_type == UNREACH ||
	    m_type == TIME_EXCEEDED ||
	    m_type == SOURCE_QUENCH) {
		/* unused */
		i.write_hton_u32 (0);
	} else if (m_type == PARAMETER_PROBLEM) {
		/* pointer */
		i.write_u8 (0);
		/* unused */
		i.write_u8 (0);
		i.write_u8 (0);
		i.write_u8 (0);
	} else if (m_type == REDIRECT) {
		i.write_hton_u32 (m_gateway.get_host_order ());
	} else if (m_type == ECHO ||
		   m_type == ECHO_REPLY) {
		i.write_hton_u16 (m_identifier);
		i.write_hton_u16 (m_seq_number);
	} else if (m_type == TIMESTAMP ||
		   m_type == TIMESTAMP_REPLY) {
		i.write_hton_u16 (m_identifier);
		i.write_hton_u16 (m_seq_number);
		i.write_hton_u32 (m_org_timestamp);
		i.write_hton_u32 (m_rx_timestamp);
		i.write_hton_u32 (m_tx_timestamp);
	} else if (m_type == INFORMATION_REQUEST ||
		   m_type == INFORMATION_REPLY) {
		i.write_hton_u16 (m_identifier);
		i.write_hton_u16 (m_seq_number);
	}	
}
void 
ChunkIcmp::peek_from (Buffer const*buffer)
{
	Buffer::Iterator i = buffer->begin ();
	m_type = i.read_u8 ();
}
void 
ChunkIcmp::remove_from (Buffer *buffer)
{
	buffer->remove_at_start (get_size ());
}

void 
ChunkIcmp::print (std::ostream *os) const
{
	*os << "(icmp) "
	    << "type=" << m_type << " "
	    << ", code=" << m_code << " "
		;
}


}; // namespace yans
