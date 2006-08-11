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

uint32_t Tags::m_prefered_size = 32;
Tags::FreeList Tags::m_free_list;

struct Tags::TagData *
Tags::allocate (uint32_t size)
{
	assert (size >= 1);
	uint32_t buf_size = size + sizeof (struct TagData) - 1;
	uint8_t *buf = new uint8_t [buf_size];
	struct TagData *data = reinterpret_cast<struct TagData *> (buf);
	data->m_size = size;
	data->m_count = 1;
	return data;
}

void
Tags::deallocate (struct TagData *data)
{
	uint8_t *buf = reinterpret_cast<uint8_t *> (data);
	delete [] buf;
}

void
Tags::recycle (struct TagData *data)
{
	if (data->m_size >= Tags::m_prefered_size) {
		Tags::m_prefered_size = data->m_size;
		Tags::m_free_list.push_back (data);
	} else {
		Tags::deallocate (data);
	}
}

struct Tags::TagData *
Tags::create (void)
{
	while (!Tags::m_free_list.empty ()) {
		struct TagData *data = Tags::m_free_list.back ();
		Tags::m_free_list.pop_back ();
		if (data->m_size > Tags::m_prefered_size) {
			data->m_count = 1;
			return data;
		}
		Tags::deallocate (data);
	}
	struct TagData *data = Tags::allocate (Tags::m_prefered_size);
	return data;
}

Tags::Tags ()
	: m_size (0),
	  m_data (Tags::create ())
{}
Tags::Tags (Tags const &o)
	: m_size (o.m_size),
	  m_data (o.m_data),
	  m_index (o.m_index)
{
	m_data->m_count++;
}
Tags &
Tags::operator = (Tags const &o)
{
	m_size = o.m_size;
	m_data = o.m_data;
	m_index = o.m_index;
	m_data->m_count++;
	return *this;
}
Tags::~Tags ()
{
	m_data->m_count--;
	if (m_data->m_count == 0) {
		Tags::recycle (m_data);
	}
#ifndef NDEBUG
	m_index.erase (m_index.begin (), m_index.end ());
#endif
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
	if (m_size + size < m_data->m_size) {
		return;
	}
	uint32_t new_size = m_data->m_size + size;
	struct TagData *data = Tags::allocate (new_size);
	memcpy (&data->m_data, &m_data->m_data, m_size);
	m_data->m_count--;
	if (m_data->m_count == 0) {
		Tags::deallocate (m_data);
	}
}

uint8_t *
Tags::get_buffer (void) const
{
	return &m_data->m_data;
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
	memcpy (get_buffer ()+m_size, tag, tag->real_get_size ());
	m_size += tag->real_get_size ();
}
void 
Tags::peek (Tag *tag) const
{
	uint32_t id = tag->real_get_id ();
	for (IndexCI i = m_index.begin (); i != m_index.end (); i++) {
		if (i->id == id) {
			memcpy (tag, get_buffer () + i->start, i->size);
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
			memcpy (tag, get_buffer () + i->start, i->size);
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
			memcpy (get_buffer () + i->start, tag, i->size);
			return;
		}
	}
	//NOTREACHED
	assert (false);
}
}; // namespace yans
