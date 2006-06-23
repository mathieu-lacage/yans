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


#include "callback.h"
#include "tcp-source.h"
#include "packet.h"
#include "host.h"
#include "tcp.h"
#include "ipv4-route.h"
#include "ipv4-end-point.h"
#include "tcp-connection.h"
#include "simulator.h"
#include "event.tcc"

#define TRACE_TCP_SOURCE 1

#ifdef TRACE_TCP_SOURCE
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP SOURCE TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_SOURCE */
# define TRACE(format,...)
#endif /* TRACE_TCP_SOURCE */

namespace yans {

TcpSource::TcpSource (Host *host)
	: m_host (host),
	  m_end_point (0)
{}
TcpSource::~TcpSource ()
{
	if (m_end_point != 0) {
		delete m_end_point;
	}
	if (m_connection != 0) {
		delete m_connection;
	}
}

void 
TcpSource::start_connect_at (Ipv4Address address, uint16_t port, double at)
{
	Simulator::schedule_abs_s (at, make_event (&TcpSource::start_connect_now, this, 
						address, port));
}
void 
TcpSource::start_disconnect_at (double at)
{
	Simulator::schedule_abs_s (at, make_event (&TcpSource::start_disconnect_now, this));
}

void 
TcpSource::start_connect_now (Ipv4Address address, uint16_t port)
{
	m_end_point->set_peer (address, port);
	m_connection = m_host->get_tcp ()->create_connection (m_end_point);
	m_connection->set_callbacks (make_callback (&TcpSource::connect_completed, this),
				     make_callback (&TcpSource::disconnect_requested, this),
				     make_callback (&TcpSource::disconnect_completed, this),
				     make_callback (&TcpSource::transmitted, this), 
				     make_callback (&TcpSource::receive, this),
				     make_callback (&TcpSource::got_ack, this));

	m_connection->start_connect ();
}
void 
TcpSource::start_disconnect_now (void)
{
	m_connection->start_disconnect ();
}
void 
TcpSource::send (PacketPtr packet)
{
	m_connection->send (packet);
}

void 
TcpSource::register_trace (TraceContainer *container)
{
	if (m_connection != 0) {
		m_connection->register_trace (container);
	}
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
TcpSource::disconnect_completed (void)
{
	delete m_end_point;
	m_end_point = 0;
	delete m_connection;
	m_connection = 0;
}
void
TcpSource::disconnect_requested (void)
{}
void
TcpSource::connect_completed (void)
{}
void 
TcpSource::receive (void)
{}
void 
TcpSource::transmitted (void)
{}
void 
TcpSource::got_ack (PacketPtr packet)
{}


}; // namespace yans
