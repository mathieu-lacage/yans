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

#include "tag-ipv4.h"

namespace yans {


uint16_t 
TagPortPair::get_dport (void)
{
	return m_dport;
}
uint16_t 
TagPortPair::get_sport (void)
{
	return m_sport;
}
void 
TagPortPair::set_dport (uint16_t dport)
{
	m_dport = dport;
}
void 
TagPortPair::set_sport (uint16_t sport)
{
	m_sport = sport;
}


Ipv4Address
TagIpv4AddressPair::get_daddress (void)
{
	return m_daddr;
}
Ipv4Address
TagIpv4AddressPair::get_saddress (void)
{
	return m_saddr;
}
void 
TagIpv4AddressPair::set_daddress (Ipv4Address daddress)
{
	m_daddr = daddress;
}
void 
TagIpv4AddressPair::set_saddress (Ipv4Address saddress)
{
	m_saddr = saddress;
}

}; // namespace yans
