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

class TcpSourceListener : public Ipv4EndPointListener {
public:
	TcpSourceListener ();
	virtual ~TcpSourceListener ();
	virtual void receive (Packet *packet);
};
TcpSourceListener::TcpSourceListener ()
{}
TcpSourceListener::~TcpSourceListener ()
{}
void TcpSourceListener::receive (Packet *packet)
{
	packet->unref ();
}


TcpSource::TcpSource (Host *host)
	: m_host (host),
	  m_end_point (0),
	  m_listener (new TcpSourceListener ()),
	  m_ref (this)
{}
TcpSource::~TcpSource ()
{
	delete m_listener;
	if (m_end_point != 0) {
		Ipv4EndPoints *end_points = m_host->get_tcp ()->get_end_points ();
		end_points->destroy (m_end_point);
	}
}
void 
TcpSource::ref (void)
{
	m_ref.ref ();
}
void 
TcpSource::unref (void)
{
	m_ref.unref ();
}
void 
TcpSource::send (Packet *packet)
{}

/* return true on success. */
bool 
TcpSource::bind (Ipv4Address address, uint16_t port)
{
	Ipv4EndPoints *end_points = m_host->get_tcp ()->get_end_points ();
	m_end_point = end_points->allocate (m_listener, address, port);
	if (m_end_point == 0) {
		return false;
	}
	return true;
}

void 
TcpSource::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_address = address;
	m_peer_port = port;
}
