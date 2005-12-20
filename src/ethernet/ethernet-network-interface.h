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

#ifndef ETHERNET_NETWORK_INTERFACE_H
#define ETHERNET_NETWORK_INTERFACE_H

#include "network-interface.h"

class Arp;
class Cable;

class EthernetNetworkInterface : public NetworkInterface {
public:
	EthernetNetworkInterface (char const *name);
	virtual ~EthernetNetworkInterface ();

	void set_mtu (uint16_t mtu);

	virtual void set_host (Host *host);
	virtual NetworkInterfaceTracer *get_tracer (void);

	virtual void set_mac_address (MacAddress self);
	virtual MacAddress get_mac_address (void);
	virtual std::string const *get_name (void);
	virtual uint16_t get_mtu (void);

	virtual void set_up   (void);
	virtual void set_down (void);
	virtual bool is_down (void);

	virtual void set_ipv4_handler (Ipv4 *ipv4);
	virtual void set_ipv4_address (Ipv4Address address);
	virtual void set_ipv4_mask    (Ipv4Mask mask);
	virtual Ipv4Address get_ipv4_address (void);
	virtual Ipv4Mask    get_ipv4_mask    (void);
	virtual Ipv4Address get_ipv4_broadcast (void);

	virtual void send (Packet *packet, Ipv4Address dest);

	void connect_to (Cable *cable);
	void recv (Packet *packet);

 private:
	void send_data (Packet *packet, MacAddress dest);
	void send_arp (Packet *packet, MacAddress dest);

	enum {
		ETHER_TYPE_IPV4 = 0x0800,
		ETHER_TYPE_ARP  = 0x0806
	};

	friend class EthernetArpMacSender;

	Arp *m_arp;
	MacAddress m_mac_address;
	Ipv4Address m_ipv4_address;
	Ipv4Mask m_ipv4_mask;
	Ipv4 *m_ipv4;
	std::string *m_name;
	bool m_up;
	Cable *m_cable;
	Host *m_host;
	NetworkInterfaceTracer *m_tracer;
	uint16_t m_mtu;
};



#endif /* ETHERNET_NETWORK_INTERFACE_H */
