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

#include "tcp-sink.h"
#include "ipv4-end-point.h"
#include "tcp-connection.h"
#include "tcp-connection-listener.h"
#include "tcp.h"
#include "host.h"
#include "gpacket.h"
#include "simulator.h"
#include "event.tcc"

#define noTRACE_TCP_SINK 1

#ifdef TRACE_TCP_SINK
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "TCP SINK TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_TCP_SINK */
# define TRACE(format,...)
#endif /* TRACE_TCP_SINK */

namespace yans {

TcpSink::TcpSink (Host *host)
	: m_host (host),
	  m_end_point (0),
	  m_real_end_point (0),
	  m_connection (0)
{
	m_host = host;
}
TcpSink::~TcpSink ()
{
	if (m_end_point != 0) {
		delete m_end_point;
		m_end_point = 0;
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
	delete m_connections;
	if (m_connection != 0) {
		delete m_connection;
	}
	if (m_real_end_point != 0) {
		delete m_real_end_point;
	}
}

void
TcpSink::receive (void)
{	
	Packet packet = m_connection->recv (m_connection->get_data_ready ());
	if (!m_callback.is_null ()) {
		m_callback (packet);
	}
}

void 
TcpSink::stop_listen_at (double at)
{
	Simulator::schedule_abs_s (at, make_event (&TcpSink::stop_listen_now, this));
}

void
TcpSink::stop_listen_now (void)
{
	if (m_end_point != 0) {
		delete m_end_point;
		m_end_point = 0;
	}
}

void 
TcpSink::register_trace (TraceContainer *container)
{
	if (m_connection != 0) {
		m_connection->register_trace (container);
	}
}

void
TcpSink::transmitted (void)
{}

void
TcpSink::got_ack (Packet packet)
{}

void
TcpSink::connect_completed (void)
{}
void
TcpSink::disconnect_requested (void)
{
	m_connection->start_disconnect ();
}
void
TcpSink::disconnect_completed (void)
{
	delete m_connection;
	m_connection = 0;
	delete m_real_end_point;
	m_real_end_point = 0;
}


bool
TcpSink::should_accept (Ipv4Address address, uint16_t port)
{
	TRACE ("connection request from " << address << ":" << port);
	if (m_connection == 0) {
		return true;
	} else {
		return false;
	}
}

void
TcpSink::connection_created (TcpConnection *connection, Ipv4EndPoint *end_point)
{
	TRACE ("connection created");
	m_connection = connection;
	m_real_end_point = end_point;
	connection->set_callbacks (make_callback (&TcpSink::connect_completed, this),
				   make_callback (&TcpSink::disconnect_requested, this),
				   make_callback (&TcpSink::disconnect_completed, this),
				   make_callback (&TcpSink::transmitted, this), 
				   make_callback (&TcpSink::receive, this),
				   make_callback (&TcpSink::got_ack, this));
}


void 
TcpSink::set_receive_callback (TcpSinkCallback callback)
{
	m_callback = callback;
}


bool 
TcpSink::bind (Ipv4Address address, uint16_t port)
{
	Tcp *tcp = m_host->get_tcp ();
	m_end_point = tcp->allocate (address, port);
	if (m_end_point == 0) {
		return false;
	}
	m_connections = tcp->create_connection_listener (m_end_point);
	m_connections->set_callbacks (make_callback (&TcpSink::should_accept, this),
				      make_callback (&TcpSink::connection_created, this));
	return true;
}

}; // namespace yans
