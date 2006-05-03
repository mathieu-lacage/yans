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
#include "tag-manager.h"
#include <cassert>

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
	: m_count (1),
	  m_buffer (new Buffer ())
{}

Packet::~Packet ()
{
	for (TagsI k = m_tags.begin (); k != m_tags.end (); k++) {
		delete (*k).second;
	}
	m_tags.erase (m_tags.begin (), m_tags.end ());
	delete m_buffer;
}

void 
Packet::ref (void) const
{
	m_count++;
}

void 
Packet::unref (void) const
{
	m_count--;
	if (m_count == 0) {
		delete this;
	}
}

Packet *
Packet::copy (void) const
{
	return copy (0, get_size ());
}
Packet *
Packet::copy (uint32_t start, uint32_t length) const
{
	assert (length <= get_size ());
	assert (start < get_size ());
	assert (start + length <= get_size ());
	Packet *other = new Packet ();
	// XXX copy tags ?? !
	Buffer *tmp = other->m_buffer;
	tmp->add_at_start (length);
	tmp->write (m_buffer->peek_data () + start, length);
	return other;
}

uint32_t 
Packet::get_size (void) const
{
	return m_buffer->get_size ();
}

void 
Packet::add_tag (uint32_t tag_id, Tag *tag)
{
	assert (get_tag (tag_id) == 0);
	m_tags.push_back (std::make_pair (tag_id, tag));
}
Tag *
Packet::get_tag (uint32_t tag_id)
{
	for (TagsI i = m_tags.begin (); i != m_tags.end (); i++) {
		if ((*i).first == tag_id) {
			return (*i).second;
		}
	}
	return 0;
}
Tag *
Packet::remove_tag (uint32_t tag_id)
{
	for (TagsI i = m_tags.begin (); i != m_tags.end (); i++) {
		if ((*i).first == tag_id) {
			Tag *tag = (*i).second;
			m_tags.erase (i);
			return tag;
		}
	}
	return 0;
}


void 
Packet::add (Chunk *chunk)
{
	chunk->add_to (m_buffer);
}
void 
Packet::add_at_end (Packet const*packet)
{
	Buffer *tmp = packet->m_buffer;
	m_buffer->add_at_end (tmp->get_size ());
	m_buffer->seek (m_buffer->get_size () - tmp->get_size ());
	m_buffer->write (tmp->peek_data (), tmp->get_size ());
}
void 
Packet::add_at_end (Packet const*packet, uint32_t start, uint32_t size)
{
	assert (packet->get_size () <= start + size);
	Buffer *tmp = packet->m_buffer;
	m_buffer->add_at_end (tmp->get_size ());
	m_buffer->seek (m_buffer->get_size () - size);
	m_buffer->write (tmp->peek_data () + start, size);
}

void 
Packet::remove (Chunk *chunk)
{
	chunk->remove_from (m_buffer);
}
void 
Packet::remove_at_end (uint32_t size)
{
	m_buffer->remove_at_end (size);
}
void
Packet::remove_at_start (uint32_t size)
{
	m_buffer->remove_at_start (size);
}

void
Packet::write (PacketReadWriteCallback callback) const
{
	uint8_t *data = m_buffer->peek_data ();
	uint32_t to_write = get_size ();
	callback (data, to_write);
}

void 
Packet::read (PacketReadWriteCallback callback, 
	      uint32_t to_read)
{
	m_buffer->add_at_start (to_read);
	uint8_t *data = m_buffer->peek_data ();
	callback (data, to_read);
}






std::ostream& operator<< (std::ostream& os, Packet const& packet)
{
	return os;
}

}; // namespace yans
