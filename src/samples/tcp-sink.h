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

class Host;
class Packet;
class TcpEndPoint;

class TcpSink {
public:
	typedef Callback<void (Packet *)> TcpSinkCallback;

	TcpSink (Host *host);
	~TcpSink ();

	void set_receive_callback (TcpSinkCallback *callback);

	bool bind (Ipv4Address address, uint16_t port);
private:
	bool should_accept (Ipv4Address from, uint16_t from_port);
	void completed (void);
	void receive (Packet *packet);
	void got_ack (Packet *packet);

	Host *m_host;
	TcpEndPoint *m_end_point;
	TcpSinkCallback *m_callback;
};

#endif /* TCP_SINK */
