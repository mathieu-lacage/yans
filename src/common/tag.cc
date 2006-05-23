/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "tag.h"
#include "strings.h"
#include <cassert>

namespace yans {

uint32_t 
TagManager::register_tag (char const *name)
{
	TagManager *self = TagManager::instance ();
	self->m_tag_id++;
	return self->m_tag_id;
}
TagManager::TagManager ()
	: m_tag_id (0)
{}
TagManager *
TagManager::instance (void)
{
	static TagManager manager;
	return &manager;
}


Tag::~Tag ()
{}


Tags::Tags ()
	: m_real_size (32),
	  m_size (0),
	  m_data (new uint8_t [32])
{}
Tags::Tags (Tags const &o)
	: m_real_size (o.m_real_size),
	  m_size (o.m_size),
	  m_data (new uint8_t [o.m_real_size])
{
	memcpy (m_data, o.m_data, o.m_size);
	m_index.insert (m_index.begin (), o.m_index.begin (), o.m_index.end ());
}
Tags::~Tags ()
{
	delete m_data;
	m_index.erase (m_index.begin (), m_index.end ());
}

Tags *
Tags::copy (void) const
{
	Tags *stags = new Tags (*this);
	return stags;
}

void
Tags::reset (void)
{
	m_size = 0;
	m_index.erase (m_index.begin (), m_index.end ());
}

void
Tags::reserve_at_end (uint32_t size)
{
	while (m_size + size > m_real_size) {
		m_real_size *= 2;
		uint8_t *data = new uint8_t [m_real_size];
		delete m_data;
		m_data = data;
	}
}

void 
Tags::add (Tag const *tag)
{
	struct Tags::IndexEntry index;
	index.id = tag->real_get_id ();
	index.size = tag->real_get_size ();
	index.start = m_size;
	m_index.push_back (index);
	reserve_at_end (tag->real_get_size ());
	memcpy (m_data+m_size, tag, tag->real_get_size ());
	m_size += tag->real_get_size ();
}
void 
Tags::peek (Tag *tag)
{
	uint32_t id = tag->real_get_id ();
	for (IndexI i = m_index.begin (); i != m_index.end (); i++) {
		if (i->id == id) {
			memcpy (tag, m_data + i->start, i->size);
			assert (tag->real_get_id () == id);
			return;
		}
	}
	//NOTREACHED
	assert (false);
}
void 
Tags::remove (Tag *tag)
{
	uint32_t id = tag->real_get_id ();
	for (IndexI i = m_index.begin (); i != m_index.end (); i++) {
		if (i->id == id) {
			memcpy (tag, m_data + i->start, i->size);
			m_index.erase (i);
			return;
		}
	}
	//NOTREACHED
	assert (false);
}
void 
Tags::update (Tag *tag)
{
	uint32_t id = tag->real_get_id ();
	for (IndexI i = m_index.begin (); i != m_index.end (); i++) {
		if (i->id == id) {
			memcpy (m_data + i->start, tag, i->size);
			return;
		}
	}
	//NOTREACHED
	assert (false);
}
}; // namespace yans
