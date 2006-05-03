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
#include <list>

namespace yans {

class Ipv4;
class Host;
class Packet;
class Ipv4EndPoint;
class Ipv4EndPoints;
class TcpConnectionListener;
class TcpConnection;
class StaticEvent;
class ChunkTcp;


enum {
/* see http://www.iana.org/assignments/protocol-numbers */
	TCP_PROTOCOL = 6
};

class Tcp {
 public:
	typedef Callback <void (void)> TcpFastTimerCallback;
	typedef Callback <void (void)> TcpSlowTimerCallback;
	Tcp ();
	virtual ~Tcp ();

	void set_host (Host *host);
	void set_ipv4 (Ipv4 *ipv4);

	Ipv4EndPoint *allocate (void);
	Ipv4EndPoint *allocate (Ipv4Address address);
	Ipv4EndPoint *allocate (Ipv4Address address, uint16_t port);
	Ipv4EndPoint *allocate (Ipv4Address local_address, uint16_t local_port,
			       Ipv4Address peer_address, uint16_t peer_port);

	TcpConnection *create_connection (Ipv4EndPoint *end_p);
	TcpConnectionListener *create_connection_listener (Ipv4EndPoint *end_p);


	// XXX
	int get_new_iss (void);
	int now (void); // tcp_now 
	int get_rexmtthresh (void); // tcp_rexmtthresh
	bool is_rfc1323 (void);

private:
	static const uint64_t FAST_TIMER_DELAY_US;
	static const uint64_t SLOW_TIMER_DELAY_US;

	typedef std::list<TcpConnection *> Connections;
	typedef std::list<TcpConnection *>::iterator ConnectionsI;

	void slow_timer (void);
	void fast_timer (void);
	void receive (Packet *packet);
	void send_reset (Packet *packet, ChunkTcp *tcp_chunk);
	void destroy_connection (TcpConnection *listener);

	Host *m_host;
	Ipv4 *m_ipv4;
	Ipv4EndPoints *m_end_p;
	Connections m_connections;
	StaticEvent *m_slow_timer;
	StaticEvent *m_fast_timer;
	bool m_running;
	int m_tcp_now;
	int m_tcp_iss;
};

}; // namespace yans

#endif /* TCP_H */
