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

#ifndef IPV4_END_POINT_H
#define IPV4_END_POINT_H

#include "callback.tcc"
#include "callback-event.tcc"
#include "ipv4-address.h"
#include <stdint.h>

namespace yans {

class Packet;

class Ipv4EndPoint {
public:
	typedef Callback<void (Packet *)> Ipv4EndPointReceptionCallback;
	typedef CallbackEvent<void (Ipv4EndPoint *)> Ipv4EndPointDestroyCallback;

	Ipv4EndPoint (Ipv4Address address, uint16_t port);
	~Ipv4EndPoint ();

	Ipv4Address get_local_address (void);
	uint16_t get_local_port (void);
	Ipv4Address get_peer_address (void);
	uint16_t get_peer_port (void);

	void set_peer (Ipv4Address address, uint16_t port);

	void receive (Packet *packet);
	void set_callback (Ipv4EndPointReceptionCallback *reception);
	void set_destroy_callback (Ipv4EndPointDestroyCallback *destroy);
private:
	Ipv4Address m_local_addr;
	uint16_t m_local_port;
	Ipv4Address m_peer_addr;
	uint16_t m_peer_port;
	Ipv4EndPointReceptionCallback *m_reception;
	Ipv4EndPointDestroyCallback *m_destroy;
};

}; // namespace yans


#endif /* IPV4_END_POINT_H */
