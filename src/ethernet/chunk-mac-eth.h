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

#ifndef CHUNK_MAC_ETH_H
#define CHUNK_MAC_ETH_H

#include "chunk.h"
#include "mac-address.h"
#include "ipv4-address.h"

namespace yans {

class ChunkMacEth : public Chunk {
 public:
	ChunkMacEth ();
	virtual ~ChunkMacEth ();

	void set_source (MacAddress source);
	void set_destination (MacAddress source);
	void set_length (uint16_t length);
	uint16_t get_length (void);
	MacAddress get_source (void);
	MacAddress get_destination (void);

private:
	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const*buffer);
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
	uint32_t get_size (void) const;
	MacAddress m_source;
	MacAddress m_destination;
	uint16_t m_length;
};

}; // namespace yans

#endif /* CHUNK_MAC_ETH_H */
