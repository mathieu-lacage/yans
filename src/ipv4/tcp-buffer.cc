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

#include "tcp-buffer.h"
#include "packet.h"


#define nopeTRACE_TCP_BUFFER 1

#ifdef TRACE_TCP_BUFFER
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP BUFFER " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_BUFFER */
# define TRACE(format,...)
#endif /* TRACE_TCP_BUFFER */


#define CHECK_STATE \
check_state ();


namespace yans {

TcpBuffer::TcpBuffer ()
{}
TcpBuffer::~TcpBuffer ()
{}

void 
TcpBuffer::check_state (void)
{
	uint32_t prev_start = 0;
	uint32_t prev_end = 0;
	TRACE ("state");
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		assert ((*i).first != 0);
		uint32_t cur_start = (*i).second;
		uint32_t cur_end = cur_start + (*i).first->get_size ();
#ifdef TRACE_TCP_PIECES
		uint32_t seq_start = m_start + cur_start;
		uint32_t seq_end = m_start + cur_end;
#endif
		TRACE ("cur start=" << cur_start << ", cur end=" << cur_end <<
		       ", seq start=" << seq_start << ", seq end=" << seq_end);
		assert (cur_start < m_size);
		assert (cur_end <= m_size);
		assert (prev_start <= cur_start);
		assert (cur_start >= prev_end);
		prev_start = cur_start;
		prev_end = cur_end;
	}	
}


void 
TcpBuffer::set_size (uint32_t size)
{
	m_size = size;
}
uint32_t 
TcpBuffer::get_size (void)
{
	return m_size;
}
void 
TcpBuffer::set_start (uint32_t start)
{
	m_start = start;
}
uint32_t 
TcpBuffer::get_current (void)
{
	return m_start;
}
uint32_t 
TcpBuffer::add_at (Packet const *packet, uint32_t offset)
{
	uint32_t trim_start; // amount to trim at start of input packet
	uint32_t trim_end;   // amount to trim at end of input packet
	uint32_t packet_start;
	uint32_t packet_end;

	if (m_start > offset) {
		trim_start = m_start - offset;
		packet_start = 0;
	} else {
		trim_start = 0;
		packet_start = offset - m_start;
	}
	packet_end  = packet_start + packet->get_size ();
	packet_end -= trim_start;
	if (packet_end > m_size) {
		trim_end = packet_end - m_size;
	} else {
		trim_end = 0;
	}
	packet_end -= trim_end;


	for (PiecesI i = m_pieces.begin (); i != m_pieces.end ();) {
		uint32_t cur_start = (*i).second;
		uint32_t cur_end = (*i).second + (*i).first->get_size ();
		if (packet_end > cur_end && packet_start < cur_start) {
			// the current piece is entirely covered
			// by the current packet.
			(*i).first->unref ();
			i = m_pieces.erase (i);
			continue;
		} else if (packet_end < cur_end && packet_start > cur_start) {
			// the current packet is entirely included
			// by the current piece.
			break;
		} 
		if (packet_start < cur_end && packet_end > cur_end) {
			trim_start += cur_end - packet_start;
			packet_start += cur_end - packet_start;
		} 
		if (packet_start < cur_start && packet_end > cur_start) {
			trim_end += packet_end - cur_start;
			packet_end -= packet_end - cur_start;
			Packet *copy = packet->copy (trim_start, packet_end);
			m_pieces.insert (i, std::make_pair (copy, packet_start));
			return copy->get_size ();
		}
		// the current packet overlaps partially the current piece
		// and the overlap has been calculated in trim_start, trim_end
		// packet_start and packet_end
		i++;
	}
	return 0;
}
uint32_t 
TcpBuffer::add_at_back (Packet const *packet)
{
	uint32_t stored = 0;
	if (m_pieces.empty ()) {
		stored = add_at (packet, m_start);
	} else {
		Piece last = *(--(m_pieces.end ()));
		stored = add_at (packet, m_start + last.second + last.first->get_size ());
	}
	return stored;
}

Packet *
TcpBuffer::get_at_front (uint32_t size)
{
	return get_at (m_start, size);
}
Packet *
TcpBuffer::get_at (uint32_t offset, uint32_t size)
{
	assert (size > 0);
	Packet *packet = new Packet ();
	uint32_t expected_start = 0;
	uint32_t end = offset + size;
	bool adding = false;
	/* this assert checks to see if we are not overflowing the uint32_t */
	assert (end >= offset + size);
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		uint32_t cur_start = (*i).second;
		uint32_t cur_end = (*i).second + (*i).first->get_size ();
		if (adding && expected_start != cur_start) {
			break;
		}
		if (cur_end <= offset || cur_start >= end) {
			/* the current piece does not overlap our target area. */
			continue;
		}
		adding = true;
		uint32_t trim_start;
		if (cur_start < offset) {
			trim_start = offset - cur_start;
		} else {
			trim_start = 0;
		}
		uint32_t piece_end;
		if (cur_end >= end) {
			piece_end = end;
			adding = false;
		} else {
			piece_end = cur_end;
		}
		packet->add_at_end ((*i).first, trim_start, piece_end - trim_start);
		if (!adding) {
			break;
		}
		expected_start = cur_end;
	}
	if (packet->get_size () == 0) {
		packet->unref ();
		return 0;
	}
	return packet;	
}


void 
TcpBuffer::remove_at_front (uint32_t size)
{
	assert (get_data_at_front () >= size);
	if (size == 0) {
		CHECK_STATE;
		return;
	}
	uint32_t expected_start = 0;
	PiecesI last = m_pieces.begin ();
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		uint32_t cur_start = (*i).second;
		uint32_t cur_size = (*i).first->get_size ();
		uint32_t cur_end = (*i).second + cur_size;

		assert (cur_start == expected_start);
		if (cur_start >= size) {
			break;
		}
		Packet *packet = (*i).first;
		if (cur_end > size) {
			assert (cur_start < size);
			packet->remove_at_start (cur_size - (cur_end - size));
			break;
		}
		expected_start = cur_end;
		packet->unref ();
		last = i;
		last++;
	}
	m_pieces.erase (m_pieces.begin (), last);

	/* Now, we need to fix the offsets recorded in the list to 
	 * account for the data removed.
	 * The amount of data removed is located in found == size.
	 */
	for (PiecesI j = m_pieces.begin (); j != m_pieces.end (); j++) {
		if ((*j).second < size) {
			(*j).second = 0;
		} else {
			(*j).second -= size;
		}
	}
	m_start += size;
	CHECK_STATE;
}


uint32_t 
TcpBuffer::get_data_at_front (void)
{
	if (m_pieces.empty ()) {
		return 0;
	}
	uint32_t expected_offset = 0;
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		if ((*i).second != expected_offset) {
			break;
		}
		expected_offset = (*i).second + (*i).first->get_size ();
	}
	return expected_offset;
}
uint32_t 
TcpBuffer::get_empty_at_back (void)
{
	if (m_pieces.empty ()) {
		return m_size;
	}
	Piece end = m_pieces.back ();
	uint32_t offset = end.second + end.first->get_size ();	
	assert (offset <= m_size);
	return m_size - offset;
}
bool 
TcpBuffer::is_empty (void)
{
	return true;
}


}; // namespace yans


#ifdef RUN_SELF_TESTS

#include "chunk-constant-data.h"

namespace yans {

Packet *
TcpBufferTest::create_one_packet (uint32_t size)
{
	Packet *packet = new Packet ();
	ChunkConstantData data (size, 0);
	packet->add (&data);
	return packet;
}
#define CHECK_FRONT_DATA(pieces, size)            \
if (!check_front_data (pieces, size, __LINE__)) { \
	ok = false;                               \
}                                                 

bool
TcpBufferTest::check_front_data (TcpBuffer *buffer, uint32_t expected_data, int line)
{
	if (buffer->get_data_at_front () != expected_data) { 
		failure () << "at line " << line << " expected: " << expected_data
			   << " got: " << buffer->get_data_at_front ()
			   << std::endl; 
		return false;
	}
	return true;
}
bool 
TcpBufferTest::run_tests (void)
{
	bool ok = true;

	TcpBuffer *buffer = new TcpBuffer ();
	buffer->set_size (100);
	Packet *piece;
	CHECK_FRONT_DATA (buffer, 0);
	piece = create_one_packet (1);
	buffer->add_at (piece, 0);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 1);

	piece = create_one_packet (1);
	buffer->add_at (piece, 0);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 1);

	piece = create_one_packet (1);
	buffer->add_at (piece, 1);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 2);

	piece = create_one_packet (1);
	buffer->add_at (piece, 0);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 2);

	piece = create_one_packet (6);
	buffer->add_at (piece, 2);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 8);

	piece = create_one_packet (3);
	buffer->add_at (piece, 3);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 8);

	piece = create_one_packet (3);
	buffer->add_at (piece, 15);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 8);

	piece = create_one_packet (6);
	buffer->add_at (piece, 8);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 14);

	piece = create_one_packet (1);
	buffer->add_at (piece, 14);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 18);

	buffer->remove_at_front (17);
	CHECK_FRONT_DATA (buffer, 1);

	piece = create_one_packet (99);
	buffer->add_at (piece, 17); // 18/17
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 99);

	/* here, we hit the buffer size limit. */
	piece = create_one_packet (2);
	buffer->add_at (piece, 116);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 100);

	buffer->set_size (102);
	piece = create_one_packet (1);
	buffer->add_at (piece, 117);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 101);

	piece = create_one_packet (99);
	buffer->add_at (piece, 117);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 102);

	piece = create_one_packet (1);
	buffer->add_at (piece, 17);
	piece->unref ();
	CHECK_FRONT_DATA (buffer, 102);

	return ok;
}

}; // namespace yans

#endif /* RUN_SELF_TESTS */
