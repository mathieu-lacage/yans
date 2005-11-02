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

#include "chunk-tcp.h"
#include "utils.h"
#include "buffer.h"

ChunkTcp::ChunkTcp ()
	: m_source_port (0),
	  m_destination_port (0),
	  m_sequence_number (0),
	  m_ack_number (0),
	  m_header_length (5),
	  m_flags (0),
	  m_window_size (0),
	  m_checksum (0),
	  m_urgent_pointer (0),
	  m_mss (0)
{}

ChunkTcp::~ChunkTcp ()
{}

void 
ChunkTcp::set_source_port (uint16_t port)
{
	m_source_port = utils_hton_16 (port);
}
void 
ChunkTcp::set_destination_port (uint16_t port)
{
	m_destination_port = utils_hton_16 (port);
}
void 
ChunkTcp::set_sequence_number (uint32_t sequence)
{
	m_sequence_number = utils_hton_32 (sequence);
}
void ChunkTcp::set_window_size (uint16_t size)
{
	m_window_size = utils_hton_16 (size);
}

uint16_t 
ChunkTcp::get_source_port (void)
{
	return utils_ntoh_16 (m_source_port);
}
uint16_t 
ChunkTcp::get_destination_port (void)
{
	return utils_ntoh_16 (m_destination_port);
}
uint32_t 
ChunkTcp::get_sequence_number (void)
{
	return utils_ntoh_32 (m_sequence_number);
}
uint32_t 
ChunkTcp::get_ack_number (void)
{
	return utils_ntoh_32 (m_ack_number);
}
uint16_t 
ChunkTcp::get_window_size (void)
{
	return utils_ntoh_16 (m_window_size);
}
uint16_t 
ChunkTcp::get_urgent_pointer (void)
{
	return utils_ntoh_16 (m_urgent_pointer);
}
uint16_t 
ChunkTcp::get_option_mss (void)
{
	return (m_mss >> 16) & 0xffff;
}
void 
ChunkTcp::enable_flag_syn (void)
{
	enable_flag (SYN);
}
void 
ChunkTcp::enable_flag_fin (void)
{
	enable_flag (FIN);
}
void 
ChunkTcp::enable_flag_rst (void)
{
	enable_flag (RST);
}
void 
ChunkTcp::enable_flag_ack (uint32_t ack)
{
	enable_flag (ACK);
	m_ack_number = utils_hton_32 (ack);
}
void 
ChunkTcp::enable_flag_urg (uint16_t ptr)
{
	enable_flag (URG);
	m_urgent_pointer = utils_hton_16 (ptr);
}
void 
ChunkTcp::enable_flag_psh (void)
{
	enable_flag (PSH);
}
void 
ChunkTcp::enable_option_mss (uint16_t mss)
{
	uint32_t real = 0;
	real = (2) | (4 << 8) | (mss << 16);
	m_mss = utils_hton_32 (real);
	m_header_length = 6;
}

void 
ChunkTcp::disable_flag_syn (void)
{
	disable_flag (SYN);
}
void 
ChunkTcp::disable_flag_fin (void)
{
	disable_flag (FIN);
}
void 
ChunkTcp::disable_flag_rst (void)
{
	disable_flag (RST);
}
void 
ChunkTcp::disable_flag_ack (void)
{
	disable_flag (ACK);
}
void 
ChunkTcp::disable_flag_urg (void)
{
	disable_flag (URG);
}
void 
ChunkTcp::disable_flag_psh (void)
{
	disable_flag (PSH);
}

bool 
ChunkTcp::is_flag_syn (void)
{
	return is_flag (SYN);
}
bool 
ChunkTcp::is_flag_fin (void)
{
	return is_flag (FIN);
}
bool 
ChunkTcp::is_flag_rst (void)
{
	return is_flag (RST);
}
bool 
ChunkTcp::is_flag_ack (void)
{
	return is_flag (ACK);
}
bool 
ChunkTcp::is_flag_urg (void)
{
	return is_flag (URG);
}
bool 
ChunkTcp::is_flag_psh (void)
{
	return is_flag (PSH);
}
bool 
ChunkTcp::is_option_mss (void)
{
	assert ((m_mss != 0)?m_header_length == 6:m_header_length == 5);
	return (m_mss != 0)?true:false;
}

bool 
ChunkTcp::is_checksum_ok (void)
{
	uint16_t checksum = calculate_checksum ((uint8_t *)&m_source_port, 20);
	if (checksum == 0xffff) {
		return true;
	} else {
		return false;
	}
}

uint32_t 
ChunkTcp::get_size (void)
{
	uint32_t size = 0;
	size += 20;
	if (is_option_mss ()) {
		size += 4;
	}
	return size;
}

Chunk *
ChunkTcp::copy (void)
{
	ChunkTcp *chunk = new ChunkTcp ();
	*chunk = *this;
	return chunk;
}

void
ChunkTcp::serialize (WriteBuffer *buffer)
{
	// XXX 
	uint16_t checksum = calculate_checksum ((uint8_t *)&m_source_port, 
						get_size ());
	buffer->write ((uint8_t *)&m_source_port, 2);
	buffer->write ((uint8_t *)&m_destination_port, 2);
	buffer->write ((uint8_t *)&m_sequence_number, 4);
	buffer->write ((uint8_t *)&m_ack_number, 4);
	buffer->write_u8 (m_header_length);
	buffer->write_u8 (m_flags);
	buffer->write ((uint8_t *)&m_window_size, 2);
	buffer->write ((uint8_t *)&checksum, 2);
	buffer->write ((uint8_t *)&m_urgent_pointer, 2);
	if (is_option_mss ()) {
		buffer->write ((uint8_t *)&m_mss, 4);
	}
}

void
ChunkTcp::deserialize (ReadBuffer *buffer)
{
	buffer->read ((uint8_t *)&m_source_port, 2);
	buffer->read ((uint8_t *)&m_destination_port, 2);
	buffer->read ((uint8_t *)&m_sequence_number, 4);
	buffer->read ((uint8_t *)&m_ack_number, 4);
	m_header_length = buffer->read_u8 ();
	m_flags = buffer->read_u8 ();
	buffer->read ((uint8_t *)&m_window_size, 2);
	buffer->read ((uint8_t *)&m_checksum, 2);
	buffer->read ((uint8_t *)&m_urgent_pointer, 2);
	if (get_size () == 24) {
		buffer->read ((uint8_t *)&m_mss, 4);
	}
}

void
ChunkTcp::print (std::ostream *os)
{
	*os << "tcp -- ";
	*os << " source port: " << m_source_port;
	*os << " destination port: " << m_destination_port;
	*os << " sequence number: " << m_sequence_number;
	if (is_flag_ack ()) {
		*os << " ack number: " << m_ack_number;
	}
	*os << " header length: " << get_size ();
	*os << " flags:";
	if (m_flags == 0) {
		*os << " nil";
	} else {
		if (is_flag_syn ()) {
			*os << " syn";
		} 
		if (is_flag_fin ()) {
			*os << "fin";
		}
		if (is_flag_rst ()) {
			*os << " rst";
		}
		if (is_flag_ack ()) {
			*os << " ack";
		}
		if (is_flag_urg ()) {
			*os << " urg";
		}
		if (is_flag_psh ()) {
			*os << " psh";
		}
	}
	*os << " window size: " << m_window_size;
	*os << " checksum: " << m_checksum;
	if (is_flag_urg ()) {
		*os << " urgent pointer: " << m_urgent_pointer;
	}
	if (is_option_mss ()) {
		*os << " mss: " << (m_mss >> 16);
	}
}


bool 
ChunkTcp::is_flag (uint8_t n)
{
	if (m_flags & (1 << n)) {
		return true;
	} else {
		return false;
	}
}
void 
ChunkTcp::enable_flag (uint8_t n)
{
	m_flags |= 1 << n;
}
void 
ChunkTcp::disable_flag (uint8_t n)
{
	m_flags &= ~(1 << n);
}
