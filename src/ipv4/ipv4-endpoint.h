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

#ifndef IPV4_ENDPOINT_H
#define IPV4_ENDPOINT_H

#include <stdint.h>
#include "ipv4-address.h"
#include "callback.tcc"
#include <list>

namespace yans {

class Packet;
class Ipv4EndPoints;

class Ipv4EndPoint {
public:
	typedef Callback<void (Packet *)> Ipv4EndPointListener;

	~Ipv4EndPoint ();
	uint16_t get_port (void);
	Ipv4Address get_address (void);
	void set_callback (Ipv4EndPointListener *callback);
	Ipv4EndPointListener *peek_callback (void);
private:
	friend class Ipv4EndPoints;
	Ipv4EndPoint (Ipv4EndPoints *end_points,
		      Ipv4Address address,
		      uint16_t port);
	Ipv4EndPoints *m_end_points;
	Ipv4EndPointListener *m_callback;
	Ipv4Address m_address;
	uint16_t m_port;
};

class Ipv4EndPoints {
public:
	Ipv4EndPoints ();
	~Ipv4EndPoints ();

	Ipv4EndPoint *allocate (void);
	Ipv4EndPoint *allocate (Ipv4Address address);
	Ipv4EndPoint *allocate (Ipv4Address address, uint16_t port);

	Ipv4EndPoint *lookup (Ipv4Address address, uint16_t port);
private:
	friend class Ipv4EndPoint;
	Ipv4EndPoint *insert (Ipv4Address address, uint16_t port);
	void destroy (Ipv4EndPoint *end_point);

	static const uint16_t PORT_RESERVED;

	typedef std::list<Ipv4EndPoint *> EndPoints;
	typedef std::list<Ipv4EndPoint *>::iterator EndPointsI;
	typedef std::list<std::pair<Ipv4Address, EndPoints *> > Addresses;
	typedef std::list<std::pair<Ipv4Address, EndPoints *> >::iterator AddressesI;
	Addresses m_addresses;
	uint16_t m_ephemeral;
};

}; // namespace yans


#endif /* IPV4_ENDPOINT_H */
