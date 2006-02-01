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

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "callback-event.tcc"
#include "ipv4-address.h"
#include <stdint.h>

namespace yans {

class Packet;
class Ipv4;
class Host;
class Ipv4EndPoint;
class Route;
class TraceContainer;


class TcpConnection {
public:
	typedef CallbackEvent<void (void)> ConnectCompletedCallback;
	typedef CallbackEvent<void (void)> DisConnectRequestedCallback;
	typedef CallbackEvent<void (void)> DisConnectCompletedCallback;
	typedef CallbackEvent<void (void)> DataReceivedCallback;
	typedef CallbackEvent<void (void)> DataTransmittedCallback;
	typedef CallbackEvent<void (Packet *)> AckReceivedCallback;
	typedef CallbackEvent<void (TcpConnection *)> TcpConnectionDestroy;

	virtual ~TcpConnection () = 0;

	virtual void set_ipv4 (Ipv4 *ipv4) = 0;
	virtual void set_host (Host *host) = 0;
	virtual void set_end_point (Ipv4EndPoint *end_point) = 0;
	virtual void set_route (Route *route) = 0;
	virtual void set_destroy_handler (TcpConnectionDestroy *handler) = 0;

	virtual void register_trace (TraceContainer *container) = 0;

	virtual void set_callbacks (ConnectCompletedCallback *connect_completed,
				    DisConnectRequestedCallback *disconnect_requested,
				    DisConnectCompletedCallback *disconnect_completed,
				    DataTransmittedCallback *data_transmitted,
				    DataReceivedCallback *data_received,
				    AckReceivedCallback *ack_received) = 0;
	virtual void start_connect (void) = 0;
	virtual void start_disconnect (void) = 0;

	virtual uint32_t get_room_left (void) = 0;
	virtual uint32_t get_data_ready (void) = 0;
	virtual uint32_t send (Packet *packet) = 0;
	virtual Packet *recv (uint32_t size) = 0;

	virtual void slow_timer (void) = 0;
	virtual void fast_timer (void) = 0;
};

}; // namespace yans


#endif /* TCP_CONNECTION_H */
