/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "udp-source.h"
#include "host.h"

namespace yapns {

UdpSource::UdpSource (SimulationContext ctx, Host *host)
{
	m_remote = ctx->peek_remote ()->create_udp_source (host->peek_remote ());
}
UdpSource::~UdpSource ()
{
	CORBA::release (m_remote);
}
void 
UdpSource::bind (Ipv4Address address, uint16_t port)
{
	m_remote->bind (address.get_host_order (), port);
}
void 
UdpSource::set_peer (Ipv4Address peer, uint16_t port)
{
	m_remote->set_peer (peer.get_host_order (), port);
}
void 
UdpSource::unbind_at (double at_s)
{
	m_remote->unbind_at_s (at_s);
}
void 
UdpSource::send (Packet *packet)
{
	assert (false);
}

::Remote::UdpSource_ptr 
UdpSource::peek_remote (void)
{
	return m_remote;
}

}; // namespace yapns
