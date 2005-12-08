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

#ifndef TCP_BSD_CONNECTION_H
#define TCP_BSD_CONNECTION_H

#include "callback.h"
#include "ipv4-address.h"
#include <stdint.h>

class Packet;
class Ipv4;
class Host;
class TcpEndPoint;
class Route;
class TcpPieces;
class ChunkTcp;


class TcpBsdConnection {
public:
	typedef Callback<void (void)> ConnectionCompletedCallback;
	typedef Callback<void (void)> DataReceivedCallback;
	typedef Callback<void (void)> DataTransmittedCallback;
	typedef Callback<void (Packet *)> AckReceivedCallback;
	typedef Callback<void (TcpBsdConnection *)> TcpBsdConnectionDestroy;

	TcpBsdConnection ();
	~TcpBsdConnection ();

	void set_ipv4 (Ipv4 *ipv4);
	void set_host (Host *host);
	void set_end_point (TcpEndPoint *end_point);
	void set_route (Route *route);
	void set_destroy_handler (TcpBsdConnectionDestroy *handler);

	void set_callbacks (ConnectionCompletedCallback *connection_completed,
			    DataTransmittedCallback *data_transmitted,
			    DataReceivedCallback *data_received,
			    AckReceivedCallback *ack_received);
	void start_connect (void);

	uint32_t get_room_left (void);
	uint32_t get_data_ready (void);
	uint32_t send (Packet *packet);
	Packet *recv (uint32_t size);

	void slow_timer (void);
	void fast_timer (void);
private:
	void notify_data_ready_to_send (void);
	void notify_room_ready_to_receive (void);
	void receive (Packet *packet);

	TcpEndPoint *m_end_point;
	Route *m_route;
	ConnectionCompletedCallback *m_connection_completed;
	DataReceivedCallback *m_data_received;
	DataTransmittedCallback *m_data_transmitted;
	AckReceivedCallback *m_ack_received;
	TcpBsdConnectionDestroy *m_destroy;
	Ipv4 *m_ipv4;
	Host *m_host;
	TcpPieces *m_send;
	TcpPieces *m_recv;
};


#endif /* TCP_BSD_CONNECTION_H */
