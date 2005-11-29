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

#include "tcp-end-point.h"

TcpEndPoint::TcpEndPoint (Ipv4Address address, uint16_t port)
	: m_local_addr (address), 
	  m_local_port (port),
	  m_peer_addr (Ipv4Address::get_any ()),
	  m_peer_port (0),
	  m_reception (0),
	  m_destroy (0)
{}
TcpEndPoint::~TcpEndPoint ()
{
	(*m_destroy) (this);
}

Ipv4Address 
TcpEndPoint::get_local_address (void)
{
	return m_local_addr;
}
uint16_t 
TcpEndPoint::get_local_port (void)
{
	return m_local_port;
}
Ipv4Address 
TcpEndPoint::get_peer_address (void)
{
	return m_peer_addr;
}
uint16_t 
TcpEndPoint::get_peer_port (void)
{
	return m_peer_port;
}
void 
TcpEndPoint::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_addr = address;
	m_peer_port = port;
}

void 
TcpEndPoint::receive (Packet *packet)
{
	(*m_reception) (packet);
}
void 
TcpEndPoint::set_callback (TcpEndPointReceptionCallback *reception)
{
	m_reception = reception;
}

void 
TcpEndPoint::set_destroy_callback (TcpEndPointDestroyCallback *destroy)
{
	m_destroy = destroy;
}
