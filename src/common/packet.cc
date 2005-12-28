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

#include "packet.h"
#include "chunk.h"
#include "buffer.h"

#define TRACE_PACKET 1

#ifdef TRACE_PACKET
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "PACKET TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_PACKET */
# define TRACE(format,...)
#endif /* TRACE_PACKET */


namespace yans {

Packet::Packet ()
	: m_ref (this)
{}

Packet::~Packet ()
{
	for (PacketDestroyNotifiersCI k = m_destroy_notifiers.begin ();
	     k != m_destroy_notifiers.end (); k++) {
		(*(*k)) (this);
		delete (*k);
	}
	m_destroy_notifiers.erase (m_destroy_notifiers.begin (),
				   m_destroy_notifiers.end ());
	for (ChunksCI i = m_chunks.begin (); i != m_chunks.end (); i++) {
		delete (*i);
	}
	m_chunks.erase (m_chunks.begin (), m_chunks.end ());
	for (TagsI k = m_tags.begin (); k != m_tags.end (); k++) {
		delete (*k).second;
	}
	m_tags.erase (m_tags.begin (), m_tags.end ());
}

void 
Packet::add_destroy_notifier (PacketDestroyNotifier *notifier)
{
	m_destroy_notifiers.push_back (notifier);
}

void 
Packet::ref (void)
{
	m_ref.ref ();
}

void 
Packet::unref (void)
{
	m_ref.unref ();
}

void 
Packet::add_tag (uint32_t tag_id, Tag *tag)
{
	assert (m_tags.find (tag_id) == m_tags.end ());
	m_tags[tag_id] = tag;
}
Tag *
Packet::get_tag (uint32_t tag_id)
{
	if (m_tags.find (tag_id) == m_tags.end ()) {
		return 0;
	} else {
		return m_tags[tag_id];
	}
}
Tag *
Packet::remove_tag (uint32_t tag_id)
{
	if (m_tags.find (tag_id) == m_tags.end ()) {
		return 0;
	} else {
		Tag *tag = m_tags[tag_id];
		m_tags.erase (m_tags.find (tag_id));
		return tag;
	}
}


void 
Packet::add_header (Chunk *header)
{
	m_chunks.push_front (header);
}
void 
Packet::add_trailer (Chunk *trailer)
{
	m_chunks.push_back (trailer);
}
Chunk *
Packet::remove_header (void)
{
	if (m_chunks.empty ()) {
		return 0;
	}
	Chunk *header = m_chunks.front ();
	m_chunks.pop_front ();
	return header;
}
Chunk *
Packet::remove_trailer (void)
{
	if (m_chunks.empty ()) {
		return 0;
	}
	Chunk *trailer = m_chunks.back ();
	m_chunks.pop_back ();
	return trailer;
}
Chunk *
Packet::peek_header (void)
{
	if (m_chunks.empty ()) {
		return 0;
	}
	return m_chunks.front ();
}
Chunk *
Packet::peek_trailer (void)
{
	if (m_chunks.empty ()) {
		return 0;
	}
	return m_chunks.back ();
}

uint32_t 
Packet::get_size (void) const
{
	uint32_t size = 0;
	for (ChunksCI i = m_chunks.begin (); i != m_chunks.end (); i++) {
		size += (*i)->get_size ();
	}
	return size;
}

uint32_t
Packet::serialize (Buffer *buffer) const
{
	uint32_t loc = 0;
	for (ChunksCI i = m_chunks.begin (); i != m_chunks.end (); i++) {
		//TRACE ("serialize init at " << loc<<" size="<<(*i)->get_size ());
		buffer->seek (loc);
		(*i)->serialize_init (buffer);
		loc += (*i)->get_size ();
	}
	ChunkSerializationState state;
	Chunk *prev_chunk = 0;
	uint32_t prev = loc;
	for (ChunksCRI j = m_chunks.rbegin (); j != m_chunks.rend (); j++) {
		Chunk *current_chunk = (*j);
		assert (prev >= current_chunk->get_size ());
		uint32_t current = prev - current_chunk->get_size ();
		//TRACE ("serialize fini at "<< current<<" size="<<current_chunk->get_size ());

		ChunksCRI tmp = j;
		tmp++;
		Chunk *next_chunk;
		uint32_t next;
		if (tmp == m_chunks.rend ()) {
			next_chunk = 0;
			next = 0;
		} else {
			next_chunk = (*tmp);
			assert (current >= next_chunk->get_size ());
			next = current - next_chunk->get_size ();
		}
		buffer->seek (current);
		state.set (next_chunk, prev_chunk, next, prev, current);
		(*j)->serialize_fini (buffer, &state);
		prev = current;
	}
	buffer->seek (get_size ());
	return get_size ();
}

void 
Packet::print (std::ostream *os) const
{
	Chunks::size_type k = 0;
	for (ChunksCI i = m_chunks.begin (); i != m_chunks.end (); i++) {
		(*i)->print (os);
		k++;
		if (k < m_chunks.size ()) {
			*os << " | ";
		}
	}
}

Packet *
Packet::copy (void)
{
	Packet *other = new Packet ();
	for (ChunksCI i = m_chunks.begin (); i != m_chunks.end (); i++) {
		Chunk *new_chunk = (*i)->copy ();
		other->add_header (new_chunk);
	}
	return other;
}



std::ostream& operator<< (std::ostream& os, Packet const& packet)
{
	packet.print (&os);
	return os;
}

}; // namespace yans
