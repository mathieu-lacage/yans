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

#include "callback.h"
#include "ipv4-address.h"
#include <stdint.h>

class Packet;
class Ipv4;
class Host;
class TcpEndPoint;
class Route;


class TcpConnection {
public:
	typedef Callback<void (void)> ConnectionCompletedCallback;
	typedef Callback<void (Packet *)> PacketReceivedCallback;
	typedef Callback<void (Packet *)> AckReceivedCallback;
	typedef Callback<void (TcpConnection *)> TcpConnectionDestroy;

	TcpConnection ();
	~TcpConnection ();

	void set_ipv4 (Ipv4 *ipv4);
	void set_host (Host *host);
	void set_end_point (TcpEndPoint *end_point);
	void set_route (Route *route);
	void set_destroy_handler (TcpConnectionDestroy *handler);

	void set_callbacks (ConnectionCompletedCallback *connection_completed,
			    PacketReceivedCallback *packet_received,
			    AckReceivedCallback *ack_received);
	void start_connect (void);

	void send (Packet *packet);

	void slow_timer (void);
	void fast_timer (void);
private:
	enum TcpState_e {
		CLOSED,
		LISTEN,
		SYN_SENT,
		SYN_RCVD,
		ESTABLISHED,
		CLOSE_WAIT,
		LAST_ACK,
		FIN_WAIT_1,
		CLOSING,
		TIME_WAIT,
		FIN_WAIT_2
	};
	void receive (Packet *packet);
	void set_state (enum TcpState_e new_state);
	bool invert_packet (Packet *packet);
	uint32_t get_isn (void);
	bool send_syn_ack (Packet *packet);
	bool send_ack (Packet *packet);

	TcpEndPoint *m_end_point;
	Route *m_route;
	ConnectionCompletedCallback *m_connection_completed;
	PacketReceivedCallback *m_packet_received;
	AckReceivedCallback *m_ack_received;
	enum TcpState_e m_state;
	Ipv4 *m_ipv4;
	Host *m_host;
	TcpConnectionDestroy *m_destroy;
};


#endif /* TCP_CONNECTION_H */
