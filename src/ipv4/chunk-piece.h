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

#ifndef CHUNK_PIECE_H
#define CHUNK_PIECE_H

#include "chunk.h"

class Packet;

class ChunkPiece: public Chunk {
public:
	ChunkPiece ();
	virtual ~ChunkPiece ();

	Packet *get_original (void);
	uint32_t get_offset (void);
	void set_original (Packet *original, uint32_t offset, uint32_t size);
	void trim_start (uint32_t delta);
	void trim_end (uint32_t delta);
	

	virtual uint32_t get_size (void) const;
	virtual Chunk *copy (void) const;
	virtual void serialize_init (Buffer *buffer) const;
	virtual void serialize_fini (Buffer *buffer,
				     ChunkSerializationState *state) const;
	virtual void print (std::ostream *os) const;

private:
	Packet *m_original;
	uint32_t m_size;
	uint32_t m_offset;
};


#endif /* CHUNK_PIECE_H */
