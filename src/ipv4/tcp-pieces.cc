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


#include "tcp-pieces.h"
#include "packet.h"
#include "chunk-piece.h"

#define nopeTRACE_TCP_PIECES 1

#ifdef TRACE_TCP_PIECES
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP PIECES " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_PIECES */
# define TRACE(format,...)
#endif /* TRACE_TCP_PIECES */


#define CHECK_STATE \
check_state ();

TcpPieces::TcpPieces ()
	: m_start (0)
{}
TcpPieces::~TcpPieces ()
{}

void 
TcpPieces::check_state (void)
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
TcpPieces::set_start (uint32_t start)
{
	m_start = start;
}
uint32_t
TcpPieces::get_current (void)
{
	return m_start;
}

void 
TcpPieces::set_size (uint32_t size)
{
	m_size = size;
}
uint32_t 
TcpPieces::get_size (void)
{
	return m_size;
}

bool
TcpPieces::is_empty (void)
{
	return m_pieces.empty ();
}


void
TcpPieces::insert_piece_at (PiecesI i, ChunkPiece *piece, uint32_t offset)
{
	if (piece->get_size () > 0) {
		m_pieces.insert (i, std::make_pair (piece, offset));
	}
}

void
TcpPieces::insert_piece_at_back (ChunkPiece *piece, uint32_t offset)
{
	if (offset + piece->get_size () > m_size) {
		/* we have to trim the input piece if it is bigger
		 * than the room we have left.
		 */
		piece->trim_end (offset + piece->get_size () - m_size);
	}
	if (piece->get_size () > 0) {
		insert_piece_at (m_pieces.end (), piece, offset);
	}
}


uint32_t
TcpPieces::add_one_at (ChunkPiece *piece, uint32_t offset)
{
	assert (piece != 0);
	int32_t delta = ((int32_t)offset) - ((int32_t)m_start);
	if (delta < 0) {
		delta = 0;
	}
	offset = (uint32_t)delta;

	if (m_pieces.empty ()) {
		insert_piece_at_back (piece, offset);
		goto done;
	}
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		uint32_t cur_start = (*i).second;
		uint32_t cur_end = cur_start + (*i).first->get_size ();
		if (cur_start > offset) {
			if (offset + piece->get_size () <= cur_start) {
				/* we fit perfectly well right before the current chunk. */
				insert_piece_at (i, piece, offset);
			} else {
				/* we need to trim the end of this piece because it
				 * overlaps the current chunk. 
				 */
				piece->trim_end (offset + piece->get_size () - cur_start);
				insert_piece_at (i, piece, offset);
			}
			goto done;
		} else {
			/* We should be located after the current chunk.
			 * Verify whether or not we overlap the current chunk.
			 */
			if (offset < cur_end) {
				/* damn, we do overlap. */
				piece->trim_start (cur_end - offset);
				offset += cur_end - offset;
			}
		}
	}
	insert_piece_at_back (piece, offset);
 done:
	if (piece->get_size () == 0) {
		CHECK_STATE;
		return 0;
	}
	CHECK_STATE;
	return piece->get_size ();
}

uint32_t 
TcpPieces::add_all_at_back (ChunkPiece const*piece)
{
	uint32_t stored = 0;
	if (m_pieces.empty ()) {
		stored = add_all_at (piece, m_start);
	} else {
		Piece end = *(--(m_pieces.end ()));
		stored = add_all_at (piece, m_start + end.second + end.first->get_size ());
	}
	return stored;
}

uint32_t
TcpPieces::add_all_at (ChunkPiece const *org, uint32_t offset)
{
	ChunkPiece *p = static_cast <ChunkPiece *> (org->copy ());
	Packet *packet = p->get_original ();
	uint32_t start = p->get_offset ();
	uint32_t end = start + p->get_size ();
	TRACE ("add all at " << offset << ", start=" << start << ", end=" << end);

	ChunkPiece *piece = static_cast <ChunkPiece *> (packet->remove_header ());
	uint32_t next_cur_start = 0;
	uint32_t next_seq_start = offset;
	while (piece != 0) {
		uint32_t cur_start = next_cur_start;
		uint32_t cur_end = cur_start + piece->get_size ();
		uint32_t seq_start = next_seq_start;
		next_cur_start = cur_end;
		TRACE ("cur start=" << cur_start << ", cur end=" << cur_end <<
		       ", seq start=" << seq_start << ", next seq start=" << next_seq_start);
		if (cur_end < start) {
			/* this piece is not part of our chunk. */
			delete piece;
			goto end;
		}
		if (cur_start >= end) {
			delete piece;
			break;
		}
		if (cur_start < start) {
			piece->trim_start (start - cur_start);
		}
		if (cur_end > end) {
			piece->trim_end (end - cur_end);
		}
		if (piece->get_size () > 0) {
			TRACE ("adding at " << seq_start);
			add_one_at (piece, seq_start);
			next_seq_start = seq_start + piece->get_size ();
		} else {
			TRACE ("null");
			delete piece;
		}
	end:
		piece = static_cast <ChunkPiece *> (packet->remove_header ());
		//TRACE ("add " << piece->get_size () << " at " << at);
	}
		
	packet->unref ();
	delete p;
	CHECK_STATE;
	return 0;
}


void
TcpPieces::remove_at_front (uint32_t size)
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
		ChunkPiece *piece = (*i).first;
		if (cur_end > size) {
			assert (cur_start < size);
			piece->trim_start (cur_size - (cur_end - size));
			break;
		}
		expected_start = cur_end;
		delete piece;
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
TcpPieces::get_data_at_front (void)
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
TcpPieces::get_empty_at_back (void)
{
	if (m_pieces.empty ()) {
		return m_size;
	}
	Piece end = m_pieces.back ();
	uint32_t offset = end.second + end.first->get_size ();	
	assert (offset <= m_size);
	return m_size - offset;
}


Packet *
TcpPieces::get_at_front (uint32_t size)
{
	return get_at (0, size);
}


Packet *
TcpPieces::get_at (uint32_t start, uint32_t size)
{
	assert (size > 0);
	Packet *packet = new Packet ();
	uint32_t expected_start = 0;
	uint32_t end = start + size;
	bool adding = false;
	/* this assert checks to see if we are not overflowing the uint32_t */
	assert (end >= start + size);
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		uint32_t cur_start = (*i).second;
		uint32_t cur_end = (*i).second + (*i).first->get_size ();
		if (adding && expected_start != cur_start) {
			break;
		}
		if (cur_end <= start || cur_start >= end) {
			/* the current piece does not overlap our target area. */
			continue;
		}
		ChunkPiece *piece = static_cast <ChunkPiece *> ((*i).first->copy ());
		packet->add_trailer (piece);
		adding = true;
		if (cur_start < start) {
			piece->trim_start (start - cur_start);
		}
		if (cur_end >= end) {
			piece->trim_end (cur_end - end);
			adding = false;
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


#ifdef RUN_SELF_TESTS

#include "chunk-fake-data.h"

TcpPiecesTest::TcpPiecesTest (TestManager *manager)
	: Test (manager)
{}
ChunkPiece *
TcpPiecesTest::create_one_piece (uint32_t size)
{
	ChunkPiece *piece = new ChunkPiece ();
	Packet *packet = new Packet ();
	ChunkFakeData *data = new ChunkFakeData (10, 0);
	packet->add_header (data);
	data = new ChunkFakeData (10, 1);
	packet->add_header (data);
	data = new ChunkFakeData (10, 3);
	packet->add_header (data);
	data = new ChunkFakeData (size, 4);
	packet->add_header (data);

	piece->set_original (packet, 1, size);

	packet->unref ();

	return piece;
}
ChunkPiece *
TcpPiecesTest::create_many_pieces (uint32_t size)
{
	uint32_t offset = 3;
	ChunkPiece *piece = new ChunkPiece ();
	Packet *packet = new Packet ();
	piece->set_original (packet, offset, size);
	ChunkPiece *p = create_one_piece (offset);
	packet->add_header (p);
	p = create_one_piece (size);
	packet->add_header (p);
	p = create_one_piece (size);
	packet->add_header (p);

	return piece;
}
#define CHECK_FRONT_DATA(pieces, size)            \
if (!check_front_data (pieces, size, __LINE__)) { \
	ok = false;                               \
}                                                 

bool
TcpPiecesTest::check_front_data (TcpPieces *pieces, uint32_t expected_data, int line)
{
	if (pieces->get_data_at_front () != expected_data) { 
		failure () << "at line " << line << " expected: " << expected_data
			   << " got: " << pieces->get_data_at_front ()
			   << std::endl; 
		return false;
	}
	return true;
}
bool 
TcpPiecesTest::run_tests (void)
{
	bool ok = true;

	TcpPieces *pieces = new TcpPieces ();
	pieces->set_size (100);
	ChunkPiece *piece;
	CHECK_FRONT_DATA (pieces, 0);
	piece = create_many_pieces (1);
	pieces->add_all_at (piece, 0);
	delete piece;
	CHECK_FRONT_DATA (pieces, 1);

	piece = create_many_pieces (1);
	pieces->add_all_at (piece, 0);
	delete piece;
	CHECK_FRONT_DATA (pieces, 1);

	piece = create_many_pieces (1);
	pieces->add_all_at (piece, 1);
	delete piece;
	CHECK_FRONT_DATA (pieces, 2);

	piece = create_many_pieces (1);
	pieces->add_all_at (piece, 0);
	delete piece;
	CHECK_FRONT_DATA (pieces, 2);

	piece = create_many_pieces (6);
	pieces->add_all_at (piece, 2);
	delete piece;
	CHECK_FRONT_DATA (pieces, 8);

	piece = create_many_pieces (3);
	pieces->add_all_at (piece, 3);
	delete piece;
	CHECK_FRONT_DATA (pieces, 8);

	piece = create_many_pieces (3);
	pieces->add_all_at (piece, 15);
	delete piece;
	CHECK_FRONT_DATA (pieces, 8);

	piece = create_many_pieces (6);
	pieces->add_all_at (piece, 8);
	delete piece;
	CHECK_FRONT_DATA (pieces, 14);

	piece = create_many_pieces (1);
	pieces->add_all_at (piece, 14);
	delete piece;
	CHECK_FRONT_DATA (pieces, 18);

	pieces->remove_at_front (17);
	CHECK_FRONT_DATA (pieces, 1);

	piece = create_many_pieces (99);
	pieces->add_all_at (piece, 17); // 18/17
	delete piece;
	CHECK_FRONT_DATA (pieces, 99);

	/* here, we hit the buffer size limit. */
	piece = create_many_pieces (2);
	pieces->add_all_at (piece, 116);
	delete piece;
	CHECK_FRONT_DATA (pieces, 100);

	pieces->set_size (102);
	piece = create_many_pieces (1);
	pieces->add_all_at (piece, 117);
	delete piece;
	CHECK_FRONT_DATA (pieces, 101);

	piece = create_many_pieces (99);
	pieces->add_all_at (piece, 117);
	delete piece;
	CHECK_FRONT_DATA (pieces, 102);

	piece = create_many_pieces (1);
	pieces->add_all_at (piece, 17);
	delete piece;
	CHECK_FRONT_DATA (pieces, 102);

	return ok;
}



#endif /* RUN_SELF_TESTS */
