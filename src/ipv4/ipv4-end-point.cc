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

#include "ipv4-end-point.h"
#include "simulator.h"
#include "event.tcc"

namespace {
void
invoke_now (yans::Ipv4EndPoint::DestroyCallback callback, yans::Ipv4EndPoint *end_point)
{
	callback (end_point);
}
};

namespace yans {

Ipv4EndPoint::Ipv4EndPoint (Ipv4Address address, uint16_t port)
	: m_local_addr (address), 
	  m_local_port (port),
	  m_peer_addr (Ipv4Address::get_any ()),
	  m_peer_port (0)
{}
Ipv4EndPoint::~Ipv4EndPoint ()
{
	Event ev = make_event (invoke_now, m_destroy, this);
	Simulator::schedule_now (ev);
}

Ipv4Address 
Ipv4EndPoint::get_local_address (void)
{
	return m_local_addr;
}
uint16_t 
Ipv4EndPoint::get_local_port (void)
{
	return m_local_port;
}
Ipv4Address 
Ipv4EndPoint::get_peer_address (void)
{
	return m_peer_addr;
}
uint16_t 
Ipv4EndPoint::get_peer_port (void)
{
	return m_peer_port;
}
void 
Ipv4EndPoint::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_addr = address;
	m_peer_port = port;
}

void 
Ipv4EndPoint::receive (Packet packet, Chunk *chunk)
{
	m_reception (packet, chunk);
}
void 
Ipv4EndPoint::set_callback (ReceptionCallback reception)
{
	m_reception = reception;
}

void 
Ipv4EndPoint::set_destroy_callback (DestroyCallback destroy)
{
	m_destroy = destroy;
}


}; // namespace yans
