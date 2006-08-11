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
#ifndef GPACKET_H
#define GPACKET_H

#include <stdint.h>
#include "gbuffer.h"
#include "chunk.h"
#include "tag.h"
#include "callback.h"

namespace yans {

class GPacket {
public:
	typedef Callback<void,uint8_t *,uint32_t> PacketReadWriteCallback;
	GPacket ();
	GPacket create_fragment (uint32_t start, uint32_t length) const;
	uint32_t get_size (void) const;
	void add (Chunk *chunk);
	void peek (Chunk *chunk) const;
	void remove (Chunk *chunk);
	void add_tag (Tag const*tag);
	void remove_tag (Tag *tag);
	void peek_tag (Tag *tag) const;
	void update_tag (Tag *tag);
	void write (PacketReadWriteCallback callback) const;
	void add_at_end (GPacket packet);
	void add_at_end (GPacket packet, uint32_t offset, uint32_t size);
	void remove_at_end (uint32_t size);
	void remove_at_start (uint32_t size);

private:
	GPacket (GBuffer buffer);
	GBuffer m_buffer;
	Tags m_tags;
};

}; // namespace yans

#endif /* GPACKET_H */
