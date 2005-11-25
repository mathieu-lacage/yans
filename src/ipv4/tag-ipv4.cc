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

uint32_t TagOutIpv4::m_tag = TagManager::instance ()->register_tag ("out-ipv4");

TagOutIpv4::TagOutIpv4 (Route const*route)
	: m_route (*route)
{}

uint16_t 
TagOutIpv4::get_dport (void)
{
	return m_dport;
}
uint16_t 
TagOutIpv4::get_sport (void)
{
	return m_sport;
}

Ipv4Address
TagOutIpv4::get_daddress (void)
{
	return m_daddress;
}
Ipv4Address
TagOutIpv4::get_saddress (void)
{
	return m_saddress;
}
void 
TagOutIpv4::set_dport (uint16_t dport)
{
	m_dport = dport;
}
void 
TagOutIpv4::set_sport (uint16_t sport)
{
	m_sport = sport;
}
void 
TagOutIpv4::set_daddress (Ipv4Address daddress)
{
	m_daddress = daddress;
}
void 
TagOutIpv4::set_saddress (Ipv4Address saddress)
{
	m_saddress = saddress;
}
Route const *
TagOutIpv4::get_route (void)
{
	return &m_route;
}

uint32_t 
TagOutIpv4::get_tag (void)
{
	return m_tag;
}



uint32_t TagInIpv4::m_tag = TagManager::instance ()->register_tag ("in-ipv4");

TagInIpv4::TagInIpv4 (NetworkInterface *interface)
	: m_interface (interface)
{}

uint16_t 
TagInIpv4::get_dport (void)
{
	return m_dport;
}
uint16_t 
TagInIpv4::get_sport (void)
{
	return m_sport;
}

Ipv4Address
TagInIpv4::get_daddress (void)
{
	return m_daddress;
}
Ipv4Address
TagInIpv4::get_saddress (void)
{
	return m_saddress;
}

void 
TagInIpv4::set_dport (uint16_t dport)
{
	m_dport = dport;
}
void 
TagInIpv4::set_sport (uint16_t sport)
{
	m_sport = sport;
}
void 
TagInIpv4::set_daddress (Ipv4Address daddress)
{
	m_daddress = daddress;
}
void 
TagInIpv4::set_saddress (Ipv4Address saddress)
{
	m_saddress = saddress;
}
NetworkInterface *
TagInIpv4::get_interface (void)
{
	return m_interface;
}

uint32_t 
TagInIpv4::get_tag (void)
{
	return m_tag;
}

