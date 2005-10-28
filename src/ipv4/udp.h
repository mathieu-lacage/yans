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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#include "transport-protocol.h"
#include "ipv4-address.h"

class Ipv4;
class Ipv4EndPoints;
class Host;

class Udp : public TransportProtocol {
public:
	Udp ();
	virtual ~Udp ();

	void set_host (Host *host);
	void set_ipv4 (Ipv4 *ipv4);

	Ipv4EndPoints *get_end_points (void);

	virtual uint8_t get_protocol (void);
	virtual void receive (Packet *packet);
	void send (Packet *packet);
 private:
	static const uint8_t UDP_PROTOCOL;
	Ipv4 *m_ipv4;
	Host *m_host;
	Ipv4EndPoints *m_end_points;
};

#endif /* UDP_H */
