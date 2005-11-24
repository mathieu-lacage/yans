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

#include "tcp.h"
#include "ipv4-endpoint.h"
#include "ipv4.h"

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t Tcp::TCP_PROTOCOL = 6;

Tcp::Tcp ()
{
	m_end_points = new Ipv4EndPoints ();
}
Tcp::~Tcp ()
{}

void 
Tcp::set_host (Host *host)
{
	m_host = host;
}
void 
Tcp::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
	m_ipv4->register_transport_protocol (make_callback (&Tcp::receive, this), 
					     TCP_PROTOCOL);
}


void 
Tcp::receive (Packet *packet)
{}

Ipv4EndPoints *
Tcp::get_end_points (void)
{
	return m_end_points;
}




class TcpIpv4EndPointListener : public Ipv4EndPointListener {
public:
	TcpIpv4EndPointListener (TcpEndPoint *tcp);
	virtual ~TcpIpv4EndPointListener ();
	virtual void receive (Packet *packet);
private:
	TcpEndPoint *m_tcp;
};

TcpIpv4EndPointListener::TcpIpv4EndPointListener (TcpEndPoint *tcp)
	: m_tcp (tcp)
{}
TcpIpv4EndPointListener::~TcpIpv4EndPointListener ()
{}
void 
TcpIpv4EndPointListener::receive (Packet *packet)
{
	m_tcp->receive (packet);
}





TcpEndPoint::TcpEndPoint ()
{
	m_state = LISTEN;
}
TcpEndPoint::~TcpEndPoint ()
{
	delete m_connection_acception;
	delete m_connection_completed;
	delete m_packet_received;
	delete m_ack_received;
}

Ipv4EndPointListener *
TcpEndPoint::get_ipv4_listener (void)
{
        return m_ipv4_listener;
}

void
TcpEndPoint::set_peer (Ipv4Address dest, uint16_t port)
{
	m_peer = dest;
	m_peer_port = port;
}
void 
TcpEndPoint::set_callbacks (ConnectionAcceptionCallback *connection_acception,
			    ConnectionCompletedCallback *connection_completed,
			    PacketReceivedCallback *packet_received,
			    AckReceivedCallback *ack_received)
{
	m_connection_acception = connection_acception;
	m_connection_completed = connection_completed;
	m_packet_received = packet_received;
	m_ack_received = ack_received;
}


void 
TcpEndPoint::set_state (enum TcpState_e new_state)
{
	m_state = new_state;
}


void
TcpEndPoint::start_connect (void)
{
	
	set_state (SYN_SENT);
}

void
TcpEndPoint::send (Packet *packet)
{}


void
TcpEndPoint::receive (Packet *packet)
{
	
}


