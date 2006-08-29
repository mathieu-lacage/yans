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

#ifndef TCP_BUFFER_H
#define TCP_BUFFER_H

#include <stdint.h>
#include <list>
#include <utility>
#include "yans/packet.h"

namespace yans {


class TcpBuffer {
public:
	TcpBuffer ();
	~TcpBuffer ();

	void set_size (uint32_t size);
	uint32_t get_size (void);
	void set_start (uint32_t start);
	uint32_t get_current (void);

	/* These functions copy the input piece, store
	 * the copy internally, and return the number of
	 * bytes copied.
	 */
	uint32_t add_at (Packet const packet, uint32_t offset);
	uint32_t add_at_back (Packet const packet);

	/* Returns 0 if no data could be found at the front
	 * of the buffer.
	 */
	Packet get_at_front (uint32_t size, bool *found);


	void remove_at_front (uint32_t size);


	uint32_t get_data_at_front (void);
	uint32_t get_empty_at_back (void);
	bool is_empty (void);

private:
	typedef std::pair<Packet , uint32_t> Piece;
	typedef std::list<Piece> Pieces;
	typedef std::list<Piece>::iterator PiecesI;

	Packet get_at (uint32_t offset, uint32_t size, bool *found);
	int32_t seq_sub (uint32_t a, uint32_t b);
	void insert_piece_at_back (Packet piece, uint32_t offset);
	void insert_piece_at (PiecesI i, Packet piece, uint32_t offset);
	void check_state (void);
	
	Pieces m_pieces;
	uint32_t m_size;
	uint32_t m_start;
};

}; //namespace yans

#endif /* TCP_BUFFER_H */
