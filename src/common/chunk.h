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

#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <ostream>

namespace yans {

class Buffer;
class Chunk;


class ChunkSerializationState {
public:
	uint32_t get_current (void);
	uint32_t get_next (void);
	uint32_t get_prev (void);
	Chunk *get_prev_chunk (void);
	Chunk *get_next_chunk (void);

	void set (Chunk *prev_chunk, Chunk *next_chunk,
		  uint32_t prev, uint32_t next,
		  uint32_t current);
private:
	uint32_t m_current;
	uint32_t m_next;
	uint32_t m_prev;
	Chunk *m_prev_chunk;
	Chunk *m_next_chunk;
};

class Chunk {
public:
	virtual ~Chunk () = 0;

	virtual uint32_t get_size (void) const = 0;
	// this is a deep copy.
	virtual Chunk *copy (void) const = 0;

	virtual void serialize_init (Buffer *buffer) const = 0;
	virtual void serialize_fini (Buffer *buffer,
				     ChunkSerializationState *state) const = 0;
	virtual void print (std::ostream *os) const = 0;
};

std::ostream& operator<< (std::ostream& os, Chunk const& chunk);

}; // namespace yans

#endif /* CHUNK_H */
