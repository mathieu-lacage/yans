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

#include <iostream>


class UdpSinkListener : public Ipv4EndPointListener {
public:
	UdpSinkListener (UdpSink *sink);
	virtual ~UdpSinkListener ();
	virtual void receive (Packet *packet);
private:
	UdpSink *m_sink;
};
UdpSinkListener::UdpSinkListener (UdpSink *sink)
	: m_sink (sink)
{}
UdpSinkListener::~UdpSinkListener ()
{}
void UdpSinkListener::receive (Packet *packet)
{
	m_sink->receive (packet);
}


UdpSink::UdpSink (Host *host)
	: m_host (host),
	  m_end_point (0),
	  m_listener (new UdpSinkListener (this)),
	  m_n_rx (0),
	  m_size_rx (0)
{}

UdpSink::~UdpSink ()
{
	if (m_end_point != 0) {
		Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
		end_points->destroy (m_end_point);
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
	delete m_listener;
	m_listener = (UdpSinkListener *)0xdeadbeaf;
}

void 
UdpSink::print_stats (void)
{
	std::cout << "packets received: " << m_n_rx << std::endl
		  << "bytes received: " << m_size_rx << std::endl;
}

bool 
UdpSink::bind (Ipv4Address address, uint16_t port)
{
	assert (m_end_point == 0);
	Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
	m_end_point = end_points->allocate (m_listener, address, port);
	if (m_end_point == 0) {
		return false;
	}
	return true;
}

void
UdpSink::receive (Packet *packet)
{
	m_host->get_tracer ()->trace_rx_app (packet);
	m_n_rx++;
	m_size_rx += packet->get_size ();	
}
