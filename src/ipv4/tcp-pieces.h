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

#ifndef TCP_PIECES_H
#define TCP_PIECES_H

#include <stdint.h>
#include <list>
#include <utility>

namespace yans {

class Packet;
class ChunkPiece;

class TcpPieces {
public:
	TcpPieces ();
	~TcpPieces ();

	void set_size (uint32_t size);
	uint32_t get_size (void);
	void set_start (uint32_t start);
	uint32_t get_current (void);

	/* These functions copy the input piece, store
	 * the copy internally, and return the number of
	 * bytes copied.
	 * The input piece should be:
	 * piece -> packet -> piece -> packet -> data
	 *             \-> piece -> packet -> data
	 *              \-> piece -> packet -> data
	 *               \-> piece -> packet -> data
	 */
	uint32_t add_all_at (ChunkPiece const* piece, uint32_t offset);
	uint32_t add_all_at_back (ChunkPiece const* piece);

	/* Returns 0 if no data could be found at the front
	 * of the buffer.
	 * The packet will look like:
	 * packet -> piece -> packet -> data
	 *    \-> piece -> packet -> data
	 *     \-> piece -> packet -> data
	 *      \-> piece -> packet -> data
	 */
	Packet *get_at_front (uint32_t size);
	Packet *get_at (uint32_t offset, uint32_t size);


	void remove_at_front (uint32_t size);


	uint32_t get_data_at_front (void);
	uint32_t get_empty_at_back (void);
	bool is_empty (void);

private:
	typedef std::pair<ChunkPiece *, uint32_t> Piece;
	typedef std::list<Piece> Pieces;
	typedef std::list<Piece>::iterator PiecesI;

	uint32_t add_one_at (ChunkPiece *piece, uint32_t offset);
	void insert_piece_at_back (ChunkPiece *piece, uint32_t offset);
	void insert_piece_at (PiecesI i, ChunkPiece *piece, uint32_t offset);
	void check_state (void);
	
	Pieces m_pieces;
	uint32_t m_size;
	uint32_t m_start;
};

}; //namespace yans

#ifdef RUN_SELF_TESTS
#include "test.h"

namespace yans {

class ChunkPiece;
class TcpPiecesTest : public Test {
public:
	virtual bool run_tests (void);
private:
	ChunkPiece *create_one_piece (uint32_t size);
	ChunkPiece *create_many_pieces (uint32_t size);
	bool check_front_data (TcpPieces *pieces, uint32_t expected_data, int line);
};

}; //namespace yans
#endif /* RUN_SELF_TESTS */


#endif /* TCP_PIECES_H */
