/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "tag-ipv4.h"

uint32_t TagIpv4::m_tag = TagManager::instance ()->register_tag ("ipv4");

uint32_t 
TagIpv4::get_tag (void)
{
	return m_tag;
}

