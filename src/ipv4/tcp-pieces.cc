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
	m_pieces.push_back (std::make_pair (piece, offset));
}


void
TcpPieces::add_at_back (ChunkPiece *piece)
{
	assert (piece->get_size () > 0);

	uint32_t offset;
	if (m_pieces.empty ()) {
		offset = 0;
	} else {
		Piece end = m_pieces.back ();
		offset = end.second + end.first->get_size ();
		assert (offset < m_size);
	}
	insert_piece_at_back (piece, offset);
}
void
TcpPieces::add_at (ChunkPiece *piece, uint32_t offset)
{
	assert (piece->get_size () > 0);

	if (m_pieces.empty ()) {
		insert_piece_at_back (piece, offset);
		return;
	}
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		if ((*i).second > offset) {
			if (offset + piece->get_size () <= (*i).second) {
				/* we fit perfectly well right before the current chunk. */
				m_pieces.insert (i, std::make_pair (piece, offset));
			} else {
				/* we need to trim the end of this piece because it
				 * overlaps the current chunk. 
				 */
				piece->trim_end (offset + piece->get_size () - (*i).second);
				m_pieces.insert (i, std::make_pair (piece, offset));
			}
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
	return;
}

void
TcpPieces::remove_at_front (uint32_t size)
{
	assert (size > 0);
	assert (!m_pieces.empty ());
	assert (get_data_at_front () >= size);
	uint32_t expected_offset = 0;
	uint32_t found = 0;
	PiecesI end = m_pieces.begin ();
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		end = i;
		if ((*i).second != expected_offset) {
			assert (expected_offset != 0);
			assert (size == get_data_at_front ());
			break;
		}
		ChunkPiece *piece = (*i).first;
		if (found + piece->get_size () > size) {
			assert (found < size);
			piece->trim_start (size - found);
			found = size;
			break;
		} else {
			found += piece->get_size ();
			expected_offset = (*i).second + piece->get_size ();
			delete piece;
		}
	}
	m_pieces.erase (m_pieces.begin (), end);

	/* Now, we need to fix the offsets recorded in the list to 
	 * account for the data removed.
	 * The amount of data removed is located in found == size.
	 */
	assert (found == size);
	for (PiecesI j = m_pieces.begin (); j != m_pieces.end (); j++) {
		(*j).second -= size;
	}
}

uint32_t
TcpPieces::get_data_at_front (void)
{
	uint32_t expected_offset = 0;
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		if ((*i).second != expected_offset) {
			break;
		}
		expected_offset = (*i).second + (*i).first->get_size ();
	}
	return expected_offset;
}


Packet *
TcpPieces::get_at_front (uint32_t size)
{
	assert (size > 0);
	assert (get_data_at_front () > 0);
	Packet *packet = new Packet ();
	uint32_t expected_offset = 0;
	uint32_t found = 0;
	for (PiecesI i = m_pieces.begin (); i != m_pieces.end (); i++) {
		if ((*i).second != expected_offset) {
			assert (expected_offset != 0);
			assert (packet->get_size () == get_data_at_front ());
			break;
		}
		ChunkPiece *piece = static_cast <ChunkPiece *> ((*i).first->copy ());
		if (found + piece->get_size () > size) {
			piece->trim_end (found + piece->get_size () - size);
			packet->add_trailer (piece);
			break;
		} else {
			packet->add_trailer (piece);
			found += piece->get_size ();
			expected_offset = (*i).second + piece->get_size ();
		}
	}
	return packet;
}
