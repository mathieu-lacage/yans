/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "tag-ipv4.h"

uint32_t TagOutIpv4::m_tag = TagManager::instance ()->register_tag ("ipv4");

TagOutIpv4::TagOutIpv4 (Route const*route, uint16_t sport, uint16_t dport)
	: m_route (*route),
	  m_sport (sport),
	  m_dport (dport)
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

