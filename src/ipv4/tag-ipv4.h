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

#ifndef TAG_IPV4_H
#define TAG_IPV4_H

#include "ipv4-address.h"
#include "ipv4-route.h"

namespace yans {

class Ipv4NetworkInterface;

struct TagOutPortPair {
	uint16_t m_sport;
	uint16_t m_dport;
};

struct TagInPortPair {
	uint16_t m_sport;
	uint16_t m_dport;
};

struct TagOutIpv4AddressPair {
	Ipv4Address m_saddr;
	Ipv4Address m_daddr;
};

struct TagInIpv4AddressPair {
	Ipv4Address m_saddr;
	Ipv4Address m_daddr;
};


}; // namespace yans

#endif /* TAG_IPV4_H */
