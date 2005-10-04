/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "tag-ipv4.h"

uint32_t TagOutIpv4::m_tag = TagManager::instance ()->register_tag ("ipv4");

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

