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


#include "tcp-source.h"
#include "packet.h"
#include "host.h"
#include "tcp.h"
#include "ipv4-route.h"
#include "tcp-end-point.h"
#include "tcp-bsd-connection.h"
#include "chunk-piece.h"


TcpSource::TcpSource (Host *host)
	: m_host (host),
	  m_end_point (0)
{}
TcpSource::~TcpSource ()
{
	if (m_end_point != 0) {
		delete m_end_point;
	}
	delete m_connection;
}
void 
TcpSource::start_connect (Ipv4Address address, uint16_t port)
{
	m_end_point->set_peer (address, port);
	m_connection = m_host->get_tcp ()->create_connection (m_end_point);
	m_connection->set_callbacks (make_callback (&TcpSource::completed, this), 
				     make_callback (&TcpSource::transmitted, this), 
				     make_callback (&TcpSource::receive, this),
				     make_callback (&TcpSource::got_ack, this));

	m_connection->start_connect ();
}
void 
TcpSource::send (Packet *packet)
{
	Packet *new_packet = new Packet ();
	ChunkPiece *piece = new ChunkPiece ();
	new_packet->add_header (piece);
	piece->set_original (packet, 0, packet->get_size ());
	m_connection->send (new_packet);
	new_packet->unref ();
}


/* return true on success. */
bool 
TcpSource::bind (Ipv4Address address, uint16_t port)
{
	Tcp *tcp = m_host->get_tcp ();
	m_end_point = tcp->allocate (address, port);
	if (m_end_point == 0) {
		return false;
	}
	return true;
}

bool 
TcpSource::should_accept (Ipv4Address from, uint16_t from_port)
{
	return false;
}
void
TcpSource::completed (void)
{}
void 
TcpSource::receive (void)
{}
void 
TcpSource::transmitted (void)
{}
void 
TcpSource::got_ack (Packet *packet)
{}
