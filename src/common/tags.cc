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
#include "tags.h"
#include "strings.h"
#include <cassert>

namespace yans {

uint32_t 
STagManager::register_tag (char const *name)
{
	STagManager *self = STagManager::instance ();
	self->m_tag_id++;
	return self->m_tag_id;
}
STagManager::STagManager ()
	: m_tag_id (0)
{}
STagManager *
STagManager::instance (void)
{
	static STagManager manager;
	return &manager;
}


STag::~STag ()
{}


STags::STags ()
	: m_real_size (32),
	  m_size (0),
	  m_data (new uint8_t [32])
{}
STags::STags (STags const &o)
	: m_real_size (o.m_real_size),
	  m_size (o.m_size),
	  m_data (new uint8_t [o.m_real_size])
{
	memcpy (m_data, o.m_data, m_size);
	m_index.insert (m_index.begin (), o.m_index.begin (), o.m_index.end ());
}
STags::~STags ()
{
	delete m_data;
}

STags *
STags::copy (void) const
{
	STags *stags = new STags (*this);
	return stags;
}

void
STags::reset (void)
{
	m_size = 0;
	m_index.erase (m_index.begin (), m_index.end ());
}

void
STags::reserve_at_end (uint32_t size)
{
	while (m_size + size > m_real_size) {
		m_real_size *= 2;
		uint8_t *data = new uint8_t [m_real_size];
		delete m_data;
		m_data = data;
	}
}

void 
STags::add (STag const *tag)
{
	struct STags::IndexEntry index;
	index.id = tag->real_get_id ();
	index.size = tag->real_get_size ();
	index.start = m_data + m_size;
	m_index.push_back (index);
	reserve_at_end (tag->real_get_size ());
	memcpy (m_data+m_size, tag, tag->real_get_size ());
	m_size += tag->real_get_size ();
}
void 
STags::peek (STag *tag)
{
	uint32_t id = tag->real_get_id ();
	for (IndexI i = m_index.begin (); i != m_index.end (); i++) {
		if (i->id == id) {
			memcpy (tag, i->start, i->size);
			return;
		}
	}
	//NOTREACHED
	assert (false);
}
void 
STags::remove (STag *tag)
{
	uint32_t id = tag->real_get_id ();
	for (IndexI i = m_index.begin (); i != m_index.end (); i++) {
		if (i->id == id) {
			uint8_t *start = i->start;
			uint32_t size = i->size;
			memcpy (tag, start, size);
			m_index.erase (i);
			return;
		}
	}
	//NOTREACHED
	assert (false);
}
void 
STags::update (STag *tag)
{
	uint32_t id = tag->real_get_id ();
	for (IndexI i = m_index.begin (); i != m_index.end (); i++) {
		if (i->id == id) {
			memcpy (i->start, tag, i->size);
			return;
		}
	}
	//NOTREACHED
	assert (false);
}
}; // namespace yans
