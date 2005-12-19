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
Chunk *
ChunkIcmp::copy (void) const
{
	ChunkIcmp *copy = new ChunkIcmp ();
	*copy = *this;
	return copy;
}
void 
ChunkIcmp::serialize (WriteBuffer *buffer)
{
	buffer->write_u8 (m_type);
	buffer->write_u8 (m_code);
	buffer->write_hton_u16 (0);
	if (m_type == UNREACH ||
	    m_type == TIME_EXCEEDED ||
	    m_type == SOURCE_QUENCH) {
		/* unused */
		buffer->write_hton_u32 (0);
	} else if (m_type == PARAMETER_PROBLEM) {
		/* pointer */
		buffer->write_u8 (0);
		/* unused */
		buffer->write_u8 (0);
		buffer->write_u8 (0);
		buffer->write_u8 (0);
	} else if (m_type == REDIRECT) {
		m_gateway.serialize (buffer);
	} else if (m_type == ECHO ||
		   m_type == ECHO_REPLY) {
		buffer->write_hton_u16 (m_identifier);
		buffer->write_hton_u16 (m_seq_number);
	} else if (m_type == TIMESTAMP ||
		   m_type == TIMESTAMP_REPLY) {
		buffer->write_hton_u16 (m_identifier);
		buffer->write_hton_u16 (m_seq_number);
		buffer->write_hton_u32 (m_org_timestamp);
		buffer->write_hton_u32 (m_rx_timestamp);
		buffer->write_hton_u32 (m_tx_timestamp);
	} else if (m_type == INFORMATION_REQUEST ||
		   m_type == INFORMATION_REPLY) {
		buffer->write_hton_u16 (m_identifier);
		buffer->write_hton_u16 (m_seq_number);		
	}
}
void 
ChunkIcmp::deserialize (ReadBuffer *buffer)
{}
void 
ChunkIcmp::print (std::ostream *os) const
{
	*os << "(icmp) "
	    << "type: " << m_type << " "
	    << "code: " << m_code << " "
		;
}
