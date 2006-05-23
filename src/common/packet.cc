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
#include "tag.h"
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
#if 1
typedef std::vector<Packet *> Packets;
typedef std::vector<Packet *>::iterator PacketsI;
static Packets g_packets;


Packet *
PacketFactory::create (void)
{
	if (g_packets.empty ()) {
		return new Packet ();
	} else {
		Packet *p = g_packets.back ();
		g_packets.pop_back ();
		p->reset ();
		return p;
	}
}

void
PacketFactory::recycle (Packet *packet)
{
	if (g_packets.size () < 2000) {
		// resurect the packet and store it.
		packet->m_count++;
		packet->m_buffer->cleanup ();
		g_packets.push_back (packet);
	} else {
		delete packet;
	}
}
#else
Packet *
PacketFactory::create (void)
{
	return new Packet ();
}

void
PacketFactory::recycle (Packet *packet)
{
	delete packet;
}
#endif

Packet::Packet ()
	: m_count (1),
	  m_buffer (new Buffer ()),
	  m_tags (new Tags ())
{}

Packet::~Packet ()
{
	delete m_buffer;
	delete m_tags;
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
		PacketFactory::recycle (const_cast<Packet*>(this));
	}
}
void
Packet::reset (void)
{
	m_buffer->reset ();
	m_tags->reset ();
}

Packet *
Packet::copy (void) const
{
	return copy (0, get_size ());
}
Packet *
Packet::copy (uint32_t start_off, uint32_t length) const
{
	assert (length <= get_size ());
	assert (start_off < get_size ());
	assert (start_off + length <= get_size ());
	Packet *other = PacketFactory::create ();
	// XXX copy tags ?? !
	Buffer *tmp = other->m_buffer;
	tmp->add_at_start (length);
	Buffer::Iterator dest, start, end;
	dest = tmp->begin ();
	start = m_buffer->begin ();
	start.next (start_off);
	end = start;
	end.next (length);
	dest.write (start, end);
	return other;
}

uint32_t 
Packet::get_size (void) const
{
	return m_buffer->get_size ();
}


void 
Packet::add_tag (Tag const*tag)
{
	m_tags->add (tag);
}
void 
Packet::remove_tag (Tag *tag)
{
	m_tags->remove (tag);
}
void 
Packet::peek_tag (Tag *tag)
{
	m_tags->peek (tag);
}
void 
Packet::update_tag (Tag *tag)
{
	m_tags->update (tag);
}



void 
Packet::add (Chunk *chunk)
{
	chunk->add_to (m_buffer);
}
void 
Packet::add_at_end (Packet const*packet)
{
	Buffer *src = packet->m_buffer;
	m_buffer->add_at_end (src->get_size ());
	Buffer::Iterator dest_start = m_buffer->end ();
	dest_start.prev (src->get_size ());
	dest_start.write (src->begin (), src->end ());
}
void 
Packet::add_at_end (Packet const*packet, uint32_t start, uint32_t size)
{
	assert (packet->get_size () <= start + size);
	Buffer *src = packet->m_buffer;
	m_buffer->add_at_end (src->get_size ());
	Buffer::Iterator dest_start = m_buffer->end ();
	dest_start.prev (size);
	Buffer::Iterator src_start = src->begin ();
	src_start.next (start);
	Buffer::Iterator src_end = src_start;
	src_end.next (size);
	dest_start.write (src_start, src_end);
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
	uint8_t *data = m_buffer->begin ().peek_data ();
	uint32_t to_write = get_size ();
	callback (data, to_write);
}

void 
Packet::read (PacketReadWriteCallback callback, 
	      uint32_t to_read)
{
	m_buffer->add_at_start (to_read);
	uint8_t *data = m_buffer->begin ().peek_data ();
	callback (data, to_read);
}






std::ostream& operator<< (std::ostream& os, Packet const& packet)
{
	return os;
}

}; // namespace yans
