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

#include "tcp-connection-listener.h"
#include "tag-ipv4.h"
#include "chunk-tcp.h"
#include "packet.h"
#include "tcp-connection.h"
#include "host.h"
#include "ipv4-route.h"
#include "tcp.h"
#include "ipv4-end-point.h"

namespace yans {

TcpConnectionListener::TcpConnectionListener ()
{}
TcpConnectionListener::~TcpConnectionListener ()
{
	delete m_acception;
	delete m_creation;
}

void 
TcpConnectionListener::receive (Packet *packet)
{
	TagInIpv4 *tag = static_cast <TagInIpv4 *> (packet->get_tag (TagInIpv4::get_tag ()));
	assert (tag != 0);
	ChunkTcp *tcp_chunk = static_cast <ChunkTcp *> (packet->peek_header ());
	if (tcp_chunk->is_flag_syn () &&
	    !tcp_chunk->is_flag_ack ()) {
		if ((*m_acception) (tag->get_saddress (), tag->get_sport ())) {
			Ipv4EndPoint *end_point = m_tcp->allocate (m_end_point->get_local_address (),
								  m_end_point->get_local_port (), 
								  tag->get_saddress (), 
								  tag->get_sport ());
			if (end_point == 0) {
				/* We cannot really allocate an endpoint here. 
				 * I think that this is a bad error here because since 
				 * we were able to receive this SYN packet, there
				 * should be no one else already registered on the requested
				 * end point Anyway, this is a moot point since we
				 * fail gracefully.
				 */
				return;
			}
			Route *route = m_host->get_routing_table ()->lookup (end_point->get_peer_address ());
			if (route == 0) {
				/* This is really weird but we have no route to the
				 * distant host. We fail gracefully here again.
				 */
				delete end_point;
				return;
			}
			TcpConnection *connection = m_host->get_tcp ()->create_connection (end_point);
			(*m_creation) (connection, end_point);
			end_point->receive (packet);
		}
	}

}

void 
TcpConnectionListener::set_callbacks (ConnectionAcception *connection_acception,
				      ConnectionCreated *connection_created)
{
	m_acception = connection_acception;
	m_creation = connection_created;
}


void 
TcpConnectionListener::set_host (Host *host)
{
	m_host = host;
}
void 
TcpConnectionListener::set_ipv4 (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
TcpConnectionListener::set_tcp (Tcp *tcp)
{
	m_tcp = tcp;
}
void 
TcpConnectionListener::set_end_point (Ipv4EndPoint *end_point)
{
	m_end_point = end_point;
	end_point->set_callback (make_callback (&TcpConnectionListener::receive, this));
}
 
}; // namespace yans
