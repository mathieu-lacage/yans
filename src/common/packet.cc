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
	: m_ref (this),
	  m_buffer (new Buffer ())
{}

Packet::~Packet ()
{
	for (TagsI k = m_tags.begin (); k != m_tags.end (); k++) {
		delete (*k).second;
	}
	m_tags.erase (m_tags.begin (), m_tags.end ());
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

Packet *
Packet::copy (void) const
{
	return copy (0, m_size);
}
Packet *
Packet::copy (uint32_t start, uint32_t length) const
{
	assert (length < m_size);
	assert (start < m_size);
	assert (start + length <= m_size);
	Packet *other = new Packet ();
	Buffer *tmp = other->m_buffer;
	tmp->allocate_at_start (length);
	tmp->write (m_buffer->peek_data () + start, m_buffer->get_size ());
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
Packet::add (Chunk *chunk)
{
	chunk->add_to (m_buffer);
}
void 
Packet::add_at_end (Packet *packet)
{
	Buffer *tmp = packet->m_buffer;
	m_buffer->add_at_end (tmp->get_size ());
	m_buffer->seek (m_buffer->get_size () - tmp->get_size ());
	m_buffer->write (tmp->peek_data (), tmp->get_size ());
}

void 
Packet::remove (Chunk *chunk)
{
	chunk->remove_from (m_buffer);
}
void 
Packet::remove_at_end (uint32_t size)
{
	m_buffer->delete_at_end (size);
}
void
Packet::remove_at_start (uint32_t size)
{
	m_buffer->delete_at_start (size);
}






std::ostream& operator<< (std::ostream& os, Packet const& packet)
{
	return os;
}

}; // namespace yans
