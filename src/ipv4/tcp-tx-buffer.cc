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

#include "tcp-tx-buffer.h"
#include "packet.h"
#include "chunk-piece.h"


TcpTxBuffer::TcpTxBuffer ()
	: m_chunk_start (0),
	  m_size (0), 
	  m_used (0)
{}

void 
TcpTxBuffer::set_size (uint32_t size)
{
	m_size = size;
}
uint32_t 
TcpTxBuffer::get_size_left (void)
{
	return m_size - m_used;
}

void 
TcpTxBuffer::add (Packet *packet)
{
	assert (get_size_left () >= packet->get_size ());
	m_used += packet->get_size ();
	m_data.push_back (packet);
}

Packet *
TcpTxBuffer::remove (uint32_t size)
{
	uint32_t needed = size;
	Packet *packet = new Packet ();
	DataI end = m_data.end ();
	for (DataI i = m_data.begin (); i != m_data.end (); i++) {
		ChunkPiece *piece = new ChunkPiece ();
		if (needed > (*i)->get_size ()) {
			piece->set_original (*i, m_chunk_start, (*i)->get_size ());
			packet->add_header (piece);
			m_chunk_start = 0;
			needed -= (*i)->get_size ();
		} else {
			piece->set_original (*i, m_chunk_start, needed);
			packet->add_header (piece);
			m_chunk_start = needed;
			end = i;
			break;
		}
	}
	m_data.erase (m_data.begin (), end);
	assert (m_used >= packet->get_size ());
	m_used -= packet->get_size ();
	return packet;
}
