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

class Packet;
class ChunkPiece;

class TcpPieces {
public:
	TcpPieces ();
	~TcpPieces ();

	void set_size (uint32_t size);
	uint32_t get_size (void);

	/* These functions copy the input piece, store
	 * the copy internally, and return the copy.
	 * If no data could be copied internally,
	 * they return 0.
	 */
	ChunkPiece *add_at_back (ChunkPiece *piece);
	ChunkPiece *add_at (ChunkPiece *piece, uint32_t offset);

	/* Returns 0 if no data could be found at the front
	 * of the buffer.
	 */
	Packet *get_at_front (uint32_t size);
	void remove_at_front (uint32_t size);

	Packet *get_at (uint32_t offset, uint32_t size);


	uint32_t get_data_at_front (void);
	uint32_t get_empty_at_back (void);
	bool is_empty (void);

private:
	typedef std::pair<ChunkPiece *, uint32_t> Piece;
	typedef std::list<Piece> Pieces;
	typedef std::list<Piece>::iterator PiecesI;

	void insert_piece_at_back (ChunkPiece *piece, uint32_t offset);
	void insert_piece_at (PiecesI i, ChunkPiece *piece, uint32_t offset);
	
	Pieces m_pieces;
	uint32_t m_size;
};


#endif /* TCP_PIECES_H */
