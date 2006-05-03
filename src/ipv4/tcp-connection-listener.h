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

#ifndef TCP_CONNECTION_LISTENER_H
#define TCP_CONNECTION_LISTENER_H

#include "callback.h"
#include "ipv4-address.h"
#include <stdint.h>

namespace yans {

class Packet;
class Host;
class Ipv4;
class Tcp;
class Ipv4EndPoint;
class TcpConnection;
class Chunk;

class TcpConnectionListener {
public:
	typedef Callback<bool, Ipv4Address, uint16_t> ConnectionAcception;
	typedef Callback<void, TcpConnection *, Ipv4EndPoint *> ConnectionCreated;
	typedef Callback<void, TcpConnectionListener *> TcpConnectionListenerDestroy;

	TcpConnectionListener ();
	~TcpConnectionListener ();

	void set_host (Host *host);
	void set_ipv4 (Ipv4 *ipv4);
	void set_tcp (Tcp *tcp);
	void set_end_point (Ipv4EndPoint *end_point);

	void receive (Packet *packet, Chunk *chunk);

	void set_callbacks (ConnectionAcception connection_acception,
			    ConnectionCreated connection_created);
private:
	ConnectionAcception m_acception;
	ConnectionCreated m_creation;
	Host *m_host;
	Ipv4 *m_ipv4;
	Tcp *m_tcp;
	Ipv4EndPoint *m_end_point;
};

}; // namespace yans

#endif /* TCP_CONNECTION_LISTENER_H */
