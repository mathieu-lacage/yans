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
#include "chunk-ipv4.h"

#define TRACE_CHUNK_TCP 1

#ifdef TRACE_CHUNK_TCP
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "CHUNK TCP TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_CHUNK_TCP */
# define TRACE(format,...)
#endif /* TRACE_CHUNK_TCP */

namespace yans {

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
	  m_has_option_mss (false),
	  m_has_option_timestamp (false),
	  m_has_option_windowscale (false)
{}

ChunkTcp::~ChunkTcp ()
{}

void 
ChunkTcp::set_source_port (uint16_t port)
{
	m_source_port = port;
}
void 
ChunkTcp::set_destination_port (uint16_t port)
{
	m_destination_port = port;
}
void 
ChunkTcp::set_sequence_number (uint32_t sequence)
{
	m_sequence_number = sequence;
}
void 
ChunkTcp::set_window_size (uint16_t size)
{
	m_window_size = size;
}
void 
ChunkTcp::set_ack_number (uint32_t ack)
{
	m_ack_number = ack;
}

uint16_t 
ChunkTcp::get_source_port (void)
{
	return m_source_port;
}
uint16_t 
ChunkTcp::get_destination_port (void)
{
	return m_destination_port;
}
uint32_t 
ChunkTcp::get_sequence_number (void)
{
	return m_sequence_number;
}
uint32_t 
ChunkTcp::get_ack_number (void)
{
	return m_ack_number;
}
uint16_t 
ChunkTcp::get_window_size (void)
{
	return m_window_size;
}
uint16_t 
ChunkTcp::get_urgent_pointer (void)
{
	return m_urgent_pointer;
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
ChunkTcp::enable_flag_ack ()
{
	enable_flag (ACK);
}
void 
ChunkTcp::enable_flag_urg (uint16_t ptr)
{
	enable_flag (URG);
	m_urgent_pointer = ptr;
}
void 
ChunkTcp::enable_flag_psh (void)
{
	enable_flag (PSH);
}

void 
ChunkTcp::disable_flags (void)
{
	m_flags = 0;
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
ChunkTcp::is_flag_syn (void) const
{
	return is_flag (SYN);
}
bool 
ChunkTcp::is_flag_fin (void) const
{
	return is_flag (FIN);
}
bool 
ChunkTcp::is_flag_rst (void) const
{
	return is_flag (RST);
}
bool 
ChunkTcp::is_flag_ack (void) const
{
	return is_flag (ACK);
}
bool 
ChunkTcp::is_flag_urg (void) const
{
	return is_flag (URG);
}
bool 
ChunkTcp::is_flag_psh (void) const
{
	return is_flag (PSH);
}



uint16_t 
ChunkTcp::get_option_mss (void) const
{
	assert (is_option_mss ());
	return m_option_mss;
}
uint32_t 
ChunkTcp::get_option_timestamp_value (void) const
{
	assert (is_option_timestamp ());
	return m_option_timestamp_value;
}
uint32_t 
ChunkTcp::get_option_timestamp_reply (void) const
{
	assert (is_option_timestamp ());
	return m_option_timestamp_reply;
}
uint8_t  
ChunkTcp::get_option_windowscale (void) const
{
	assert (is_option_windowscale ());
	return m_option_windowscale;
}
void 
ChunkTcp::enable_option_mss (uint16_t mss)
{
	m_has_option_mss = true;
	m_option_mss = mss;
}
void 
ChunkTcp::enable_option_timestamp (uint32_t value, uint32_t reply)
{
	m_has_option_timestamp = true;
	m_option_timestamp_value = value;
	m_option_timestamp_reply = reply;
}
void 
ChunkTcp::enable_option_windowscale (uint8_t log_scale)
{
	m_has_option_windowscale = true;
	m_option_windowscale = log_scale;
}
bool 
ChunkTcp::is_option_mss (void) const
{
	return m_has_option_mss;
}
bool 
ChunkTcp::is_option_timestamp (void) const
{
	return m_has_option_timestamp;
}
bool 
ChunkTcp::is_option_windowscale (void) const
{
	return m_has_option_windowscale;
}


bool 
ChunkTcp::is_checksum_ok (void)
{
	return true;
}

uint32_t 
ChunkTcp::get_size (void) const
{
	uint32_t size = 0;
	size += 20;
	if (is_option_mss ()) {
		size += 4;
	}
	if (is_option_windowscale ()) {
		size += 3;
	}
	if (is_option_timestamp ()) {
		size += 10;
	}
	while ((size % 4) != 0) {
		size++;
	}
	return size;
}

Chunk *
ChunkTcp::copy (void) const
{
	ChunkTcp *chunk = new ChunkTcp ();
	*chunk = *this;
	return chunk;
}

void 
ChunkTcp::serialize_init (Buffer *buffer) const
{
	uint32_t start = buffer->get_current ();
	buffer->write_hton_u16 (m_source_port);
	buffer->write_hton_u16 (m_destination_port);
	buffer->write_hton_u32 (m_sequence_number);
	buffer->write_hton_u32 (m_ack_number);
	uint8_t header_length = ((get_size () / 4) & 0x0f) << 4;
	buffer->write_u8 (header_length);
	uint8_t flags = 0;
	if (is_flag_urg ()) {
		flags |= (1<<5);
	}
	if (is_flag_ack ()) {
		flags |= (1<<4);
	}
	if (is_flag_psh ()) {
		flags |= (1<<3);
	}
	if (is_flag_rst ()) {
		flags |= (1<<2);
	}
	if (is_flag_syn ()) {
		flags |= (1<<1);
	}
	if (is_flag_fin ()) {
		flags |= (1<<0);
	}
	buffer->write_u8 (flags);
	buffer->write_hton_u16 (m_window_size);
	buffer->write_hton_u16 (0);
	buffer->write_hton_u16 (m_urgent_pointer);
	if (is_option_mss ()) {
		buffer->write_u8 (2);
		buffer->write_u8 (4);
		buffer->write_hton_u16 (m_option_mss);
	}
	if (is_option_windowscale ()) {
		buffer->write_u8 (3);
		buffer->write_u8 (3);
		buffer->write_u8 (m_option_windowscale);
	}
	if (is_option_timestamp ()) {
		buffer->write_u8 (8);
		buffer->write_u8 (10);
		buffer->write_hton_u32 (m_option_timestamp_value);
		buffer->write_hton_u32 (m_option_timestamp_reply);
	}
	uint32_t padding = get_size () - (buffer->get_current () - start);
	while (padding > 0) {
		buffer->write_u8 (0);
		padding--;
	}
	TRACE ("wrote until "<< buffer->get_current ());
}

void 
ChunkTcp::serialize_fini (Buffer *buffer,
			  ChunkSerializationState *state) const
{
	uint32_t saved0, saved1, saved2;
	buffer->skip (-12);
	saved0 = buffer->read_u32 ();
	saved1 = buffer->read_u32 ();
	saved2 = buffer->read_u32 ();
	buffer->skip (-12);
	ChunkIpv4 *ipv4 = static_cast<ChunkIpv4 *> (state->get_prev_chunk ());
	ipv4->get_source ().serialize (buffer);
	ipv4->get_destination ().serialize (buffer);
	buffer->write_u8 (0);
	buffer->write_u8 (ipv4->get_protocol ());
	buffer->write_hton_u16 (ipv4->get_payload_size ());
	buffer->skip (-12);
	uint16_t checksum = utils_checksum_calculate (buffer->peek_data () + buffer->get_current (), 
						      12 + ipv4->get_payload_size ());
	buffer->write_u32 (saved0);
	buffer->write_u32 (saved1);
	buffer->write_u32 (saved2);
	buffer->skip (+16);
	buffer->write_u16 (checksum);
}

void
ChunkTcp::print (std::ostream *os) const
{
	*os << "tcp -- ";
	*os << " source port=" << m_source_port;
	*os << ", destination port=" << m_destination_port;
	*os << ", sequence number=" << m_sequence_number;
	if (is_flag_ack ()) {

	}
	*os << ", header length=" << get_size ();
	*os << ", window size=" << m_window_size;
	*os << " flags=[";
	if (is_flag_syn ()) {
		*os << " SYN";
	} 
	if (is_flag_fin ()) {
		*os << "FIN";
	}
	if (is_flag_rst ()) {
		*os << " RST";
	}
	if (is_flag_ack ()) {
		*os << " ACK=" << m_ack_number;
	}
	if (is_flag_urg ()) {
		*os << " URG=" << m_urgent_pointer;
	}
	if (is_flag_psh ()) {
		*os << " PSH";
	}
	*os << "]";
	if (is_option_mss ()) {
		*os << ", mss=" << m_option_mss;
	}
	if (is_option_timestamp ()) {
		*os << ", tsval=" << m_option_timestamp_value
		    << ", tsecho=" << m_option_timestamp_reply;
	}
	if (is_option_windowscale ()) {
		*os << ", windowscale=" << m_option_windowscale;
	}
}


bool 
ChunkTcp::is_flag (uint8_t n) const
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

}; // namespace yans
