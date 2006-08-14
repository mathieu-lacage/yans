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


#ifndef TCP_SINK
#define TCP_SINK

#include <stdint.h>
#include "ipv4-address.h"
#include "callback.h"
#include "gpacket.h"

namespace yans {

class Host;
class Ipv4EndPoint;
class TcpConnection;
class TcpConnectionListener;
class TraceContainer;

class TcpSink {
public:
	typedef Callback<void, GPacket > TcpSinkCallback;

	TcpSink (Host *host);
	~TcpSink ();

	void set_receive_callback (TcpSinkCallback callback);

	bool bind (Ipv4Address address, uint16_t port);

	void stop_listen_at (double at);
	void register_trace (TraceContainer *container);
private:
	bool should_accept (Ipv4Address from, uint16_t from_port);
	void connection_created (TcpConnection *connection, Ipv4EndPoint *end_point);
	void connect_completed (void);
	void disconnect_requested (void);
	void disconnect_completed (void);
	void receive (void);
	void transmitted (void);
	void got_ack (GPacket packet);
	void stop_listen_now (void);

	Host *m_host;
	Ipv4EndPoint *m_end_point;
	Ipv4EndPoint *m_real_end_point;
	TcpConnectionListener *m_connections;
	TcpConnection *m_connection;
	TcpSinkCallback m_callback;
};

}; // namespace yans

#endif /* TCP_SINK */
