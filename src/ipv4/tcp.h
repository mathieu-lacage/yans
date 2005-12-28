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
#include "callback.tcc"
#include <list>

namespace yans {

class Ipv4;
class Host;
class Packet;
class TcpEndPoint;
class TcpConnectionListener;
class TcpConnection;
class Event;


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

	TcpEndPoint *allocate (void);
	TcpEndPoint *allocate (Ipv4Address address);
	TcpEndPoint *allocate (Ipv4Address address, uint16_t port);
	TcpEndPoint *allocate (Ipv4Address local_address, uint16_t local_port,
			       Ipv4Address peer_address, uint16_t peer_port);

	TcpConnection *create_connection (TcpEndPoint *end_p);
	TcpConnectionListener *create_connection_listener (TcpEndPoint *end_p);


	// XXX
	int get_new_iss (void);
	int now (void); // tcp_now 
	int get_rexmtthresh (void); // tcp_rexmtthresh
	bool is_rfc1323 (void);

private:
	static const uint64_t FAST_TIMER_DELAY_US;
	static const uint64_t SLOW_TIMER_DELAY_US;

	typedef std::list<TcpEndPoint *> TcpEndPoints;
	typedef std::list<TcpEndPoint *>::iterator TcpEndPointsI;
	typedef std::list<TcpConnection *> Connections;
	typedef std::list<TcpConnection *>::iterator ConnectionsI;

	void slow_timer (void);
	void fast_timer (void);
	bool lookup_port_local (uint16_t port);
	bool lookup_local (Ipv4Address addr, uint16_t port);
	uint16_t allocate_ephemeral_port (void);
	TcpEndPoint *lookup (Ipv4Address daddr, 
			     uint16_t dport, 
			     Ipv4Address saddr, 
			     uint16_t sport);
	void receive (Packet *packet);
	void send_reset (Packet *packet);
	void destroy_end_point (TcpEndPoint *end_point);
	void destroy_connection (TcpConnection *listener);

	Host *m_host;
	Ipv4 *m_ipv4;
	TcpEndPoints m_end_p;
	uint16_t m_ephemeral;
	Connections m_connections;
	Event *m_slow_timer;
	Event *m_fast_timer;
	bool m_running;
	int m_tcp_now;
	int m_tcp_iss;
};

}; // namespace yans

#endif /* TCP_H */
