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

#include "tag-manager.h"

TagManager *TagManager::m_instance = 0;

TagManager::TagManager ()
	: m_tag_id (1)
{}

TagManager *
TagManager::instance (void)
{
	if (m_instance == 0) {
		m_instance = new TagManager ();
	}
	return m_instance;
}

void 
TagManager::destroy (void)
{
	delete this;
	m_instance = 0;
}

uint32_t 
TagManager::register_tag (char const *name)
{
	uint32_t tag = m_tag_id;
	m_tag_mapping[tag] = name;
	m_tag_id++;
	return tag;
}

char const *
TagManager::lookup_tag (uint32_t tag_id)
{
	if (m_tag_mapping.find (tag_id) != m_tag_mapping.end ()) {
		return m_tag_mapping[tag_id];
	} else {
		return 0;
	}
}
