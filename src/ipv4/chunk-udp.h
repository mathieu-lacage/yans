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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#define CHUNK_UDP_H

#include <stdint.h>
#include "udp.h"
#include "chunk.h"

class ChunkUdp : public Chunk {
public:
	ChunkUdp ();
	virtual ~ChunkUdp ();

	void set_destination (uint16_t port);
	void set_source (uint16_t port);
	uint16_t get_source (void);
	uint16_t get_destination (void);

	void set_payload_size (uint16_t size);

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	uint16_t m_source_port;
	uint16_t m_destination_port;
	uint16_t m_udp_length;
};

#endif /* CHUNK_UDP */
