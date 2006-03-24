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

#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <ostream>
#include <list>
#include <utility>

#include "callback.tcc"

namespace yans {

class Chunk;
class Buffer;
class Tag;


class Packet {
public:
	typedef Callback<void (uint8_t *, uint32_t)> PacketReadWriteCallback;
	Packet ();
	~Packet ();

	void ref (void) const;
	void unref (void) const;
	Packet *copy (void) const;
	Packet *copy (uint32_t start, uint32_t length) const;

	uint32_t get_size (void) const;

	void add_tag (uint32_t tag_id, Tag *tag);
	Tag *get_tag (uint32_t tag_id);
	Tag *remove_tag (uint32_t tag_id);
	
	void add (Chunk *chunk);
	void add_at_end (Packet const*packet);
	void add_at_end (Packet const*packet, uint32_t offset, uint32_t size);
	void remove (Chunk *chunk);
	void remove_at_end (uint32_t size);
	void remove_at_start (uint32_t size);

	void write (PacketReadWriteCallback *callback) const;
	void read (PacketReadWriteCallback *callback, uint32_t to_read);

 private:
	typedef std::list<std::pair<uint32_t, Tag *> >Tags;
	typedef std::list<std::pair<uint32_t, Tag *> >::iterator TagsI;
	Tags m_tags;
	mutable uint32_t m_count;
	Buffer *m_buffer;
};

std::ostream& operator<< (std::ostream& os, Packet const& packet);

}; // namespace yans

#endif /* PACKET_H */
