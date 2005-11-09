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
#include "ref-count.tcc"

class TrafficAnalyzer;
class Host;
class Ipv4EndPoint;
class TcpSinkListener;
class Packet;

class TcpSink {
public:
	TcpSink (Host *host);
	~TcpSink ();
	void ref (void);
	void unref (void);

	void set_analyzer (TrafficAnalyzer *analyzer);

	bool bind (Ipv4Address address, uint16_t port);
private:
	friend class TcpSinkListener;
	void receive (Packet *packet);

	Host *m_host;
	Ipv4EndPoint *m_end_point;
	TrafficAnalyzer *m_analyzer;
	TcpSinkListener *m_listener;
	RefCount<TcpSink> m_ref;
};

#endif /* TCP_SINK */
