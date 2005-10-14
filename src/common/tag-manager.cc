/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

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
