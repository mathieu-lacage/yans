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

#include "tcp-bsd-connection.h"
#include "host.h"
#include "tcp-end-point.h"
#include "tag-ipv4.h"
#include "packet.h"
#include "chunk-tcp.h"
#include "ipv4-route.h"
#include "simulator.h"
#include "tcp.h"
#include "ipv4.h"
#include "tcp-pieces.h"
#include "chunk-piece.h"


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


#define TRACE_TCP_BSD_CONNECTION 1

#ifdef TRACE_TCP_BSD_CONNECTION
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP CONN " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_BSD_CONNECTION */
# define TRACE(format,...)
#endif /* TRACE_TCP_BSD_CONNECTION */



TcpBsdConnection::TcpBsdConnection ()
{}
TcpBsdConnection::~TcpBsdConnection ()
{
	delete m_connection_completed;
	delete m_data_received;
	delete m_data_transmitted;
	delete m_ack_received;
	if (m_destroy != 0) {
		(*m_destroy) (this);
	}
	delete m_destroy;
}

void 
TcpBsdConnection::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
TcpBsdConnection::set_host (Host *host)
{
	m_host = host;
}
void 
TcpBsdConnection::set_end_point (TcpEndPoint *end_point)
{
	m_end_point = end_point;
	m_end_point->set_callback (make_callback (&TcpBsdConnection::receive, this));
}
void 
TcpBsdConnection::set_route (Route *route)
{
	m_route = route;
}
void 
TcpBsdConnection::set_destroy_handler (TcpBsdConnectionDestroy *handler)
{
	m_destroy = handler;
}

void 
TcpBsdConnection::set_callbacks (ConnectionCompletedCallback *connection_completed,
			      DataTransmittedCallback *data_transmitted,
			      DataReceivedCallback *data_received,
			      AckReceivedCallback *ack_received)
{
	m_connection_completed = connection_completed;
	m_data_received = data_received;
	m_data_transmitted = data_transmitted;
	m_ack_received = ack_received;
}

uint32_t 
TcpBsdConnection::get_room_left (void)
{
	return m_send->get_empty_at_back ();
}

uint32_t 
TcpBsdConnection::get_data_ready (void)
{
	return m_recv->get_data_at_front ();
}


uint32_t
TcpBsdConnection::send (Packet *packet)
{
	uint32_t sent = 0;
	ChunkPiece *piece = static_cast <ChunkPiece *> (packet->remove_header ());
	while (piece != 0) {
		ChunkPiece *copy = m_send->add_at_back (piece);
		if (copy == 0) {
			/* there was no room at all to put this piece
			 * in the buffer. */
			packet->add_header (piece);
			break;
		}
		sent += copy->get_size ();
		assert (copy->get_size () <= piece->get_size ());
		if (copy->get_size () < piece->get_size ()) {
			/* there was not enough room to put this
			 * piece entirely in the buffer. */
			piece->trim_start (copy->get_size ());
			packet->add_header (piece);
			break;
		}
		delete piece;
		piece = static_cast <ChunkPiece *> (packet->remove_header ());
	}
	notify_data_ready_to_send ();
	return sent;
}
Packet *
TcpBsdConnection::recv (uint32_t size)
{
	Packet *packet = m_recv->get_at_front (size);
	if (packet == 0) {
		return 0;
	}
	m_recv->remove_at_front (packet->get_size ());
	notify_room_ready_to_receive ();
	return packet;
}

void
TcpBsdConnection::receive (Packet *packet)
{}


void
TcpBsdConnection::start_connect (void)
{}

void 
TcpBsdConnection::slow_timer (void)
{}
void 
TcpBsdConnection::fast_timer (void)
{}



void
TcpBsdConnection::notify_data_ready_to_send (void)
{}

void
TcpBsdConnection::notify_room_ready_to_receive (void)
{}


