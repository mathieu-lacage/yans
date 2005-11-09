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
#include "ipv4-endpoint.h"
#include "tcp.h"
#include "host.h"
#include "traffic-analyzer.h"


class TcpSinkListener : public Ipv4EndPointListener {
public:
	TcpSinkListener (TcpSink *sink);
	virtual ~TcpSinkListener ();
	virtual void receive (Packet *packet);
private:
	TcpSink *m_sink;
};
TcpSinkListener::TcpSinkListener (TcpSink *sink)
	: m_sink (sink)
{}
TcpSinkListener::~TcpSinkListener ()
{}
void TcpSinkListener::receive (Packet *packet)
{
	m_sink->receive (packet);
}


TcpSink::TcpSink (Host *host)
	: m_ref (this)
{
	m_host = host;
}
TcpSink::~TcpSink ()
{
	if (m_end_point != 0) {
		Ipv4EndPoints *end_points = m_host->get_tcp ()->get_end_points ();
		end_points->destroy (m_end_point);
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
	delete m_listener;
	m_listener = (TcpSinkListener *)0xdeadbeaf;
	if (m_analyzer != 0) {
		m_analyzer->unref ();
	}
}
void 
TcpSink::ref (void)
{
	m_ref.ref ();
}
void 
TcpSink::unref (void)
{
	m_ref.unref ();
}

void
TcpSink::receive (Packet *packet)
{
}

void 
TcpSink::set_analyzer (TrafficAnalyzer *analyzer)
{
	m_analyzer = analyzer;
}

bool 
TcpSink::bind (Ipv4Address address, uint16_t port)
{
	assert (m_end_point == 0);
	Ipv4EndPoints *end_points = m_host->get_tcp ()->get_end_points ();
	m_end_point = end_points->allocate (m_listener, address, port);
	if (m_end_point == 0) {
		return false;
	}
	return true;
}
