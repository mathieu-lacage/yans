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
#ifndef TAGS_H
#define TAGS_H

#include <stdint.h>

namespace yans {

struct TagData;

class Tags {
public:
	inline Tags ();
	inline Tags (Tags const &o);
	inline Tags &operator = (Tags const &o);
	inline ~Tags ();

	template <typename T>
	void add (T const*tag);

	template <typename T>
	bool remove (T *tag);

	template <typename T>
	bool peek (T *tag) const;

	template <typename T>
	bool update (T const*tag);

	inline void remove_all_tags (void);

	enum {
		SIZE = 16
	};
private:
	struct TagData {
		struct TagData *m_next;
		uint32_t m_id;
		uint32_t m_count;
		uint8_t m_data[Tags::SIZE];
	};

	void add (uint8_t const*buffer, uint32_t size, uint32_t id);
	bool remove (uint32_t id);
	bool peek (uint8_t *buffer, uint32_t size, uint32_t id) const;
	bool update (uint8_t const*buffer, uint32_t size, uint32_t id);
	struct Tags::TagData *alloc_data (void);
	void free_data (struct TagData *data);

	static struct Tags::TagData *g_free;
	static uint32_t g_n_free;

	struct TagData *m_next;
};

}; // namespace yans


#include <cassert>
#include "uid-factory.h"

namespace yans {

template <typename T>
class TypeUid {
public:
	static uint32_t get_uid (void);
private:
	T real_type;
};
template <typename T>
uint32_t TypeUid<T>::get_uid (void)
{
	static uint32_t uid = UidFactory::create ();
	return uid;
}


template <typename T>
void 
Tags::add (T const*tag)
{
	assert (sizeof (T) <= Tags::SIZE);
	uint8_t const*buf = reinterpret_cast<uint8_t const*> (tag);
	add (buf, sizeof (T), TypeUid<T>::get_uid ());
}

template <typename T>
bool
Tags::remove (T *tag)
{
	assert (sizeof (T) <= Tags::SIZE);
	return remove (TypeUid<T>::get_uid ());
}

template <typename T>
bool
Tags::peek (T *tag) const
{
	assert (sizeof (T) <= Tags::SIZE);
	uint8_t *buf = reinterpret_cast<uint8_t *> (tag);
	return peek (buf, sizeof (T), TypeUid<T>::get_uid ());
}

template <typename T>
bool
Tags::update (T const*tag)
{
	assert (sizeof (T) <= Tags::SIZE);
	uint8_t const*buf = reinterpret_cast<uint8_t const*> (tag);
	return update (buf, sizeof (T), TypeUid<T>::get_uid ());
}


Tags::Tags ()
	: m_next ()
{}

Tags::Tags (Tags const &o)
	: m_next (o.m_next)
{
	if (m_next != 0) {
		m_next->m_count++;
	}
}

Tags &
Tags::operator = (Tags const &o)
{
	// self assignment
	if (m_next == o.m_next) {
		return *this;
	}
	remove_all_tags ();
	m_next = o.m_next;
	if (m_next != 0) {
		m_next->m_count++;
	}
	return *this;
}

Tags::~Tags ()
{
	remove_all_tags ();
}

void
Tags::remove_all_tags (void)
{
	struct TagData *prev = 0;
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		cur->m_count--;
		if (cur->m_count > 0) {
			break;
		}
		if (prev != 0) {
			free_data (prev);
		}
		prev = cur;
	}
	if (prev != 0) {
		free_data (prev);
	}
	m_next = 0;
}


}; // namespace yans

#endif /* TAGS_H */
