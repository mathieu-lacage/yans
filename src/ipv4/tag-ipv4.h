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

#ifndef TAG_IPV4_H
#define TAG_IPV4_H

#include "tag-manager.h"
#include "ipv4-address.h"
#include "ipv4-route.h"

class NetworkInterface;

class TagOutIpv4 : public Tag {
public:
	TagOutIpv4 (Route const *route);

	uint16_t get_dport (void);
	uint16_t get_sport (void);
	Ipv4Address get_daddress (void);
	Ipv4Address get_saddress (void);

	void set_dport (uint16_t dport);
	void set_sport (uint16_t sport);
	void set_daddress (Ipv4Address daddress);
	void set_saddress (Ipv4Address saddress);

	Route const *get_route (void);

	static uint32_t get_tag (void);
 private:
	Route m_route;
	Ipv4Address m_daddress;
	Ipv4Address m_saddress;
	uint16_t m_sport;
	uint16_t m_dport;
	static uint32_t m_tag;
};

class TagInIpv4 : public Tag {
public:
	TagInIpv4 (NetworkInterface *interface);

	NetworkInterface *get_interface (void);
	uint16_t get_dport (void);
	uint16_t get_sport (void);
	Ipv4Address get_daddress (void);

	void set_dport (uint16_t dport);
	void set_sport (uint16_t sport);
	void set_daddress (Ipv4Address daddress);

	static uint32_t get_tag (void);
public:	
	NetworkInterface *m_interface;
	Ipv4Address m_daddress;
	uint16_t m_sport;
	uint16_t m_dport;
	static uint32_t m_tag;
};

#endif /* TAG_IPV4_H */
