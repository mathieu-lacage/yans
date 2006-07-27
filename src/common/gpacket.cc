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
#include "gpacket.h"
#include "packet.h"
#include <cassert>

namespace yans {
GPacket::GPacket ()
	: m_packet (Packet::create ())
{}
GPacket::GPacket (GPacket const &o)
{
	o.m_packet->m_count++;
	m_packet = o.m_packet;
}
GPacket::~GPacket ()
{
	m_packet->m_count--;
	if (m_packet->m_count == 0) {
		m_packet->destroy ();
	}
}	
GPacket &
GPacket::operator = (GPacket const &o)
{
	m_packet->m_count--;
	if (m_packet->m_count == 0) {
		m_packet->destroy ();
	}
	o.m_packet->m_count++;
	m_packet = o.m_packet;
}
GPacket 
GPacket::create_fragment (uint32_t start, uint32_t length) const
{
	return GPacket (m_packet->copy (start, length));
}
uint32_t 
GPacket::get_size (void) const
{
	return m_packet->get_size ();
}
void 
GPacket::add_tag (Tag const*tag)
{
	if (m_packet->m_count > 1) {
		m_packet->m_count--;
		m_packet = m_packet->copy ();
	}
	m_packet->add_tag (tag);
}
void 
GPacket::remove_tag (Tag *tag)
{
	if (m_packet->m_count > 1) {
		m_packet->m_count--;
		m_packet = m_packet->copy ();
	}
	m_packet->remove_tag (tag);
}
void 
GPacket::peek_tag (Tag *tag) const
{
	m_packet->peek_tag (tag);
}
void 
GPacket::update_tag (Tag *tag)
{
	if (m_packet->m_count > 1) {
		m_packet->m_count--;
		m_packet = m_packet->copy ();
	}
	m_packet->update_tag (tag);
}
void 
GPacket::add (Chunk *chunk)
{
	if (m_packet->m_count > 1) {
		m_packet->m_count--;
		m_packet = m_packet->copy ();
	}
	m_packet->add (chunk);
}
void 
GPacket::peek (Chunk *chunk) const
{
	m_packet->peek (chunk);
}
void 
GPacket::remove (Chunk *chunk)
{
	if (m_packet->m_count > 1) {
		m_packet->m_count--;
		m_packet = m_packet->copy ();
	}
	m_packet->remove (chunk);
}

}; // namespace yans
