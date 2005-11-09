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
}


uint8_t 
Tcp::get_protocol (void)
{
  return TCP_PROTOCOL;
}
void 
Tcp::receive (Packet *packet)
{}

Ipv4EndPoints *
Tcp::get_end_points (void)
{
	return m_end_points;
}
