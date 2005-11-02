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

#ifndef LOOPBACK_INTERFACE_H
#define LOOPBACK_INTERFACE_H

#include <stdint.h>

#include "network-interface.h"
#include "ipv4-address.h"

class LoopbackInterface : public NetworkInterface {
public:
	LoopbackInterface ();
	virtual ~LoopbackInterface ();

	virtual void set_host (Host *host);
	virtual NetworkInterfaceTracer *get_tracer (void);

	virtual uint16_t get_mtu (void);
	virtual std::string const *get_name (void);
	virtual void set_mac_address (MacAddress self);
	virtual MacAddress get_mac_address (void);

	virtual void set_up   (void);
	virtual void set_down (void);
	virtual bool is_down (void);

	virtual void set_ipv4_handler (Ipv4 *ipv4);
	virtual void set_ipv4_address (Ipv4Address address);
	virtual void set_ipv4_mask    (Ipv4Mask mask);
	virtual Ipv4Address get_ipv4_address (void);
	virtual Ipv4Mask    get_ipv4_mask    (void);

	virtual void send (Packet *packet, Ipv4Address to);
private:
	std::string m_name;
	Ipv4 *m_ipv4;
	Ipv4Address m_address;
	Ipv4Mask m_mask;
	uint16_t m_mtu;
	bool m_down;
};


#endif /* LOOPBACK_INTERFACE_H */
