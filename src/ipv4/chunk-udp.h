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

#ifndef CHUNK_UDP_H
#define CHUNK_UDP_H

#include <stdint.h>
#include "chunk.h"
#include "ipv4-address.h"

namespace yans {

class ChunkUdp : public Chunk {
public:
	ChunkUdp ();
	virtual ~ChunkUdp ();

	void set_destination (uint16_t port);
	void set_source (uint16_t port);
	uint16_t get_source (void) const;
	uint16_t get_destination (void) const;

	void set_payload_size (uint16_t size);

	void initialize_checksum (Ipv4Address source, 
				  Ipv4Address destination,
				  uint8_t protocol);

	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	uint32_t get_size (void) const;

	uint16_t m_source_port;
	uint16_t m_destination_port;
	uint16_t m_payload_size;
	uint16_t m_initial_checksum;
};

}; // namespace yans

#endif /* CHUNK_UDP */
