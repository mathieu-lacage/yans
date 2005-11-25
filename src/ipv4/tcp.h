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


#ifndef TCP_H
#define TCP_H

#include "ipv4-address.h"
#include "callback.h"

class Ipv4;
class Host;
class Ipv4EndPoints;
class Ipv4EndPoint;
class Packet;
class TcpEndPoint;

class Tcp {
 public:
	Tcp ();
	virtual ~Tcp ();

	void set_host (Host *host);
	void set_ipv4 (Ipv4 *ipv4);

	TcpEndPoint *allocate (void);
	TcpEndPoint *allocate (Ipv4Address address);
	TcpEndPoint *allocate (Ipv4Address address, uint16_t port);
	void destroy (TcpEndPoint *end_point);
private:
	void receive (Packet *packet);

	static const uint8_t TCP_PROTOCOL;
	Ipv4EndPoints *m_end_points;
	Host *m_host;
	Ipv4 *m_ipv4;
};


class TcpEndPoint {
public:
	typedef Callback<bool (Ipv4Address, uint16_t)> ConnectionAcceptionCallback;
	typedef Callback<void (void)> ConnectionCompletedCallback;
	typedef Callback<void (Packet *)> PacketReceivedCallback;
	typedef Callback<void (Packet *)> AckReceivedCallback;

	TcpEndPoint ();
	~TcpEndPoint ();

	void set_ipv4 (Ipv4 *ipv4);
	void set_ipv4_end_point (Ipv4EndPoint *end_point);

	void set_peer (Ipv4Address dest, uint16_t port);
	void set_callbacks (ConnectionAcceptionCallback *connection_acception,
			    ConnectionCompletedCallback *connection_completed,
			    PacketReceivedCallback *packet_received,
			    AckReceivedCallback *ack_received);
	void start_connect (void);

	void send (Packet *packet);
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

	Ipv4Address m_peer;
	uint16_t m_peer_port;
	ConnectionAcceptionCallback *m_connection_acception;
	ConnectionCompletedCallback *m_connection_completed;
	PacketReceivedCallback *m_packet_received;
	AckReceivedCallback *m_ack_received;
	enum TcpState_e m_state;
	Ipv4EndPoint *m_ipv4_end_point;
	Ipv4 *m_ipv4;
};



#endif /* TCP_H */
