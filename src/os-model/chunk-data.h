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

#ifndef CHUNK_DATA_H
#define CHUNK_DATA_H

#include "chunk.h"

class ChunkData : public Chunk {
public:
	ChunkData (uint8_t const *buffer, uint32_t len);
	virtual ~ChunkData ();

	virtual uint32_t get_size (void) const;
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	uint8_t *m_data;
	uint32_t m_len;
};

#endif /* CHUNK_DATA_H */
