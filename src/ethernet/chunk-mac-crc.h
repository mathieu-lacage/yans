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

#ifndef CHUNK_MAC_CRC_H
#define CHUNK_MAC_CRC_H

#include "yans/chunk.h"
#include "yans/mac-address.h"
#include "yans/ipv4-address.h"

namespace yans {

class ChunkMacCrc : public Chunk {
 public:
	ChunkMacCrc ();
	virtual ~ChunkMacCrc ();

	void set_pad (uint8_t pad_size);
private:
	virtual void print (std::ostream *os) const;
	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const *buffer);
	virtual void remove_from (Buffer *buffer);
	uint32_t get_size (void) const;
	uint8_t m_pad_size;
};

}; // namespace yans

#endif /* CHUNK_MAC_CRC_H */
