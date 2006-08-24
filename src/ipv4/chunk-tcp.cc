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
#include "yans/utils.h"
#include <cassert>

#define TRACE_CHUNK_TCP 1

#ifdef TRACE_CHUNK_TCP
#include <iostream>
#include "yans/simulator.h"
# define TRACE(x) \
std::cout << "CHUNK TCP TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_CHUNK_TCP */
# define TRACE(format,...)
#endif /* TRACE_CHUNK_TCP */

namespace yans {

bool ChunkTcp::m_calc_checksum = false;

ChunkTcp::ChunkTcp ()
	: m_source_port (0),
	  m_destination_port (0),
	  m_sequence_number (0),
	  m_ack_number (0),
	  m_header_length (5),
	  m_flags (0),
	  m_window_size (0),
	  m_urgent_pointer (0),
	  m_has_option_mss (0),
	  m_has_option_timestamp (0),
	  m_has_option_windowscale (),
	  m_initial_checksum (0)
{}

ChunkTcp::~ChunkTcp ()
{}

void
ChunkTcp::enable_checksums (void)
{
	m_calc_checksum = true;
}

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
	m_has_option_mss = 1;
	m_option_mss = mss;
}
void 
ChunkTcp::enable_option_timestamp (uint32_t value, uint32_t reply)
{
	m_has_option_timestamp = 1;
	m_option_timestamp_value = value;
	m_option_timestamp_reply = reply;
}
void 
ChunkTcp::enable_option_windowscale (uint8_t log_scale)
{
	m_has_option_windowscale = 1;
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
	return m_is_checksum_ok?true:false;
}

uint32_t
ChunkTcp::get_raw_size (void) const
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
	return size;
}

uint32_t 
ChunkTcp::get_size (void) const
{
	return get_raw_size () + get_padding ();
}
uint32_t
ChunkTcp::get_padding (void) const
{
	uint32_t raw_size = get_raw_size ();
	uint32_t size = raw_size;
	while ((size % 4) != 0) {
		size++;
	}
	return size - raw_size;
}
void 
ChunkTcp::initialize_checksum (Ipv4Address source, 
			       Ipv4Address destination,
			       uint8_t protocol,
			       uint16_t payload_size)
{
	if (m_calc_checksum) {
		uint8_t buf[12];
		source.serialize (buf);
		destination.serialize (buf+4);
		buf[8] = 0;
		buf[9] = protocol;
		uint16_t udp_length = payload_size + get_size ();
		buf[10] = udp_length >> 8;
		buf[11] = udp_length & 0xff;
		m_initial_checksum = utils_checksum_calculate (0, buf, 12);
	}
}


void
ChunkTcp::print (std::ostream *os) const
{
	*os << "tcp -- ";
	*os << " source port=" << m_source_port;
	*os << ", destination port=" << m_destination_port;
	*os << ", sequence number=" << m_sequence_number;
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

void ChunkTcp::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	i.write_hton_u16 (m_source_port);
	i.write_hton_u16 (m_destination_port);
	i.write_hton_u32 (m_sequence_number);
	i.write_hton_u32 (m_ack_number);
	uint8_t header_length = ((get_size () / 4) & 0x0f) << 4;
	i.write_u8 (header_length);
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
	i.write_u8 (flags);
	i.write_hton_u16 (m_window_size);
	i.write_hton_u16 (0);
	i.write_hton_u16 (m_urgent_pointer);
	if (is_option_mss ()) {
		i.write_u8 (2);
		i.write_u8 (4);
		i.write_hton_u16 (m_option_mss);
	}
	if (is_option_windowscale ()) {
		i.write_u8 (3);
		i.write_u8 (3);
		i.write_u8 (m_option_windowscale);
	}
	if (is_option_timestamp ()) {
		i.write_u8 (8);
		i.write_u8 (10);
		i.write_hton_u32 (m_option_timestamp_value);
		i.write_hton_u32 (m_option_timestamp_reply);
	}
	uint32_t padding = get_padding ();
	i.write_u8 (0, padding);

	if (m_calc_checksum) {
		uint16_t checksum;
		checksum = utils_checksum_calculate (m_initial_checksum, 
						     buffer->peek_data (),
						     buffer->get_size ());
		checksum = utils_checksum_complete (checksum);
		i = buffer->begin ();
		i.next (16);
		i.write_u16 (checksum);
	}
}
void ChunkTcp::peek_from (Buffer const *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	m_source_port = i.read_ntoh_u16 ();
	m_destination_port = i.read_ntoh_u16 ();
	m_sequence_number = i.read_ntoh_u32 ();
	m_ack_number = i.read_ntoh_u32 ();
        uint8_t header_length = i.read_u8 () >> 4;
	uint8_t flags = i.read_u8 ();
	if (flags & (1<<0)) {
		enable_flag (FIN);
	}
	if (flags & (1<<1)) {
		enable_flag (SYN);
	}
	if (flags & (1<<2)) {
		enable_flag (RST);
	}
	if (flags & (1<<3)) {
		enable_flag (PSH);
	}
	if (flags & (1<<4)) {
		enable_flag (ACK);
	}
	if (flags & (1<<5)) {
		enable_flag (URG);
	}
	m_window_size = i.read_ntoh_u16 ();
	i.next (2);
	m_urgent_pointer = i.read_ntoh_u16 ();
	assert (header_length >= 5);
	int32_t options_length = (header_length - 5) * 4;
	while (options_length > 0) {
		uint8_t type = i.read_u8 ();
		options_length--;
		switch (type) {
		case 0: // end-of-opt
			goto out;
			break;
		case 1: // no-op
			break;
		case 2: {
			uint8_t length = i.read_u8 ();
			options_length -= length;
			if (length != 4) {
				goto out;
			}
			uint16_t mss = i.read_ntoh_u16 ();
			enable_option_mss (mss);
		} break;
		case 3: {
			uint8_t length = i.read_u8 ();
			options_length -= length;
			if (length != 3) {
				goto out;
			}
			uint8_t scale = i.read_u8 ();
			enable_option_windowscale (scale);
		} break;
		case 8: {
			uint8_t length = i.read_u8 ();
			options_length -= length;
			if (length != 10) {
				goto out;
			}
			uint32_t value = i.read_ntoh_u32 ();
			uint32_t reply = i.read_ntoh_u32 ();
			enable_option_timestamp (value, reply);
			break;
		}
		default:
			uint8_t length = i.read_u8 ();
			options_length -= length;
			while (length > 0) {
				i.read_u8 ();
				length--;
			}
			break;
		}
	}

	if (m_calc_checksum) {
		uint16_t checksum;
		checksum = utils_checksum_calculate (m_initial_checksum, 
						     buffer->peek_data (),
						     buffer->get_size ());
		checksum = utils_checksum_complete (checksum);
		if (checksum == 0) {
			m_is_checksum_ok = 1;
		} else {
			m_is_checksum_ok = 0;
		}
	}
	
 out:
	return;
}
void ChunkTcp::remove_from (Buffer *buffer)
{
	buffer->remove_at_start (get_size ());
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
