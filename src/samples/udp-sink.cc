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

#include "udp-sink.h"
#include "udp.h"
#include "host.h"
#include "ipv4-endpoint.h"
#include "packet.h"
#include "host-tracer.h"
#include "reception-listener.h"

#include <iostream>



UdpSink::UdpSink (Host *host)
	: m_host (host),
	  m_end_point (0),
	  m_callback ()
{}

UdpSink::~UdpSink ()
{
	if (m_end_point != 0) {
		delete m_end_point;
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
	delete m_callback;
}

void 
UdpSink::set_receive_callback (UdpSinkCallback *callback)
{
	assert (m_callback == 0);
	m_callback = callback;
}


bool 
UdpSink::bind (Ipv4Address address, uint16_t port)
{
	assert (m_end_point == 0);
	Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
	m_end_point = end_points->allocate (address, port);
	m_end_point->set_callback (make_callback (&UdpSink::receive, this));
	if (m_end_point == 0) {
		return false;
	}
	return true;
}

void
UdpSink::receive (Packet *packet)
{
	m_host->get_tracer ()->trace_rx_app (packet);
	if (m_callback != 0) {
		(*m_callback) (packet);
	}
}
