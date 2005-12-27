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
#include <vector>
#include <list>

#include "sgi-hashmap.h"
#include "tag-manager.h"
#include "ref-count.tcc"

class Chunk;
class Buffer;
class Packet;

class PacketDestroyNotifier {
public:
	virtual ~PacketDestroyNotifier ();
	virtual void notify (Packet *packet) = 0;
};

class Packet {
public:
	Packet ();
	~Packet ();

	void ref (void);
	void unref (void);

	void add_destroy_notifier (PacketDestroyNotifier *notifier);

	void add_tag (uint32_t tag_id, Tag *tag);
	Tag *get_tag (uint32_t tag_id);
	Tag *remove_tag (uint32_t tag_id);

	void add_header (Chunk *header);
	void add_trailer (Chunk *trailer);
	Chunk *remove_header (void);
	Chunk *remove_trailer (void);
	Chunk *peek_header (void);
	Chunk *peek_trailer (void);

	uint32_t get_size (void) const;

	// returns the number of bytes written in the buffer.
	uint32_t serialize (Buffer *buffer) const;

	void print (std::ostream *os) const;

	Packet *copy (void);
 private:
	typedef std::list<Chunk *> Chunks;
	typedef std::list<Chunk *>::const_iterator ChunksCI;
	typedef std::list<Chunk *>::const_reverse_iterator ChunksCRI;
	typedef std::vector<PacketDestroyNotifier *> PacketDestroyNotifiers;
	typedef std::vector<PacketDestroyNotifier *>::const_iterator PacketDestroyNotifiersCI;
	typedef Sgi::hash_map<uint32_t, Tag *> Tags;
	typedef Sgi::hash_map<uint32_t, Tag *>::iterator TagsI;
	PacketDestroyNotifiers m_destroy_notifiers;
	Chunks m_chunks;
	Tags m_tags;
	RefCount<Packet> m_ref;
};

std::ostream& operator<< (std::ostream& os, Packet const& packet);

#endif /* PACKET_H */
