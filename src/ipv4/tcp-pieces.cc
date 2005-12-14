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

TcpPieces::TcpPieces ()
{}
TcpPieces::~TcpPieces ()
{}

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
		assert (piece->get_size () > 0);
	}
	insert_piece_at (m_pieces.end (), piece, offset);
}


ChunkPiece *
TcpPieces::add_at_back (ChunkPiece *piece)
{
	assert (piece->get_size () > 0);
	if (get_empty_at_back () == 0) {
		return 0;
	}

	uint32_t offset;
	if (m_pieces.empty ()) {
		offset = 0;
	} else {
		Piece end = m_pieces.back ();
		offset = end.second + end.first->get_size ();
	}
	assert (offset < m_size);
	ChunkPiece *new_piece = static_cast <ChunkPiece *> (piece->copy ());
	insert_piece_at_back (new_piece, offset);
	return new_piece;
}
ChunkPiece *
TcpPieces::add_at (ChunkPiece *org, uint32_t offset)
{
	assert (org->get_size () > 0);
	assert (offset < m_size);

	ChunkPiece *piece = static_cast <ChunkPiece *> (org->copy ());
	if (m_pieces.empty ()) {
		insert_piece_at_back (piece, offset);
		return piece;
	}
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		if ((*i).second > offset) {
			if (offset + piece->get_size () <= (*i).second) {
				/* we fit perfectly well right before the current chunk. */
				insert_piece_at (i, piece, offset);
			} else {
				/* we need to trim the end of this piece because it
				 * overlaps the current chunk. 
				 */
				piece->trim_end (offset + piece->get_size () - (*i).second);
				insert_piece_at (i, piece, offset);
			}
			break;
		} else {
			/* We should be located after the current chunk.
			 * Verify whether or not we overlap the current chunk.
			 */
			if (offset < (*i).second + (*i).first->get_size ()) {
				/* damn, we do overlap. */
				piece->trim_start ((*i).second + (*i).first->get_size () - offset);
			}
		}
	}
	if (piece->get_size () == 0) {
		delete piece;
		return 0;
	}
	return piece;
}

void
TcpPieces::remove_at_front (uint32_t size)
{
	assert (get_data_at_front () >= size);
	if (size == 0) {
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
		(*j).second -= size;
	}
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
		if (cur_end > end) {
			piece->trim_end (cur_end - end);
			adding = false;
			break;
		}
		expected_start = cur_end;
	}
	if (packet->get_size () == 0) {
		delete packet;
		return 0;
	}
	return packet;	
}
