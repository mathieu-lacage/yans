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

	inline void remove_all (void);

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
	template <typename T>
	class TypeUid {
	public:
		static const uint32_t get_uid (void);
	private:
		T real_type;
	};
	class UidFactory {
	public:
		static uint32_t create (void);
	};


	bool remove (uint32_t id);
	bool update (uint8_t const*buffer, uint32_t id);
	struct Tags::TagData *alloc_data (void);
	void free_data (struct TagData *data);

	static struct Tags::TagData *g_free;
	static uint32_t g_n_free;

	struct TagData *m_next;
};

}; // namespace yans


#include <cassert>
#include <string.h>

namespace yans {

template <typename T>
const uint32_t Tags::TypeUid<T>::get_uid (void)
{
	static const uint32_t uid = UidFactory::create ();
	return uid;
}


template <typename T>
void 
Tags::add (T const*tag)
{
	assert (sizeof (T) <= Tags::SIZE);
	uint8_t const*buf = reinterpret_cast<uint8_t const*> (tag);
	// ensure this id was not yet added
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		assert (cur->m_id != Tags::TypeUid<T>::get_uid ());
	}
	struct TagData *new_start = alloc_data ();
	new_start->m_count = 1;
	new_start->m_next = 0;
	new_start->m_id = Tags::TypeUid<T>::get_uid ();
	memcpy (new_start->m_data, buf, sizeof (T));
	new_start->m_next = m_next;
	m_next = new_start;
}

template <typename T>
bool
Tags::remove (T *tag)
{
	assert (sizeof (T) <= Tags::SIZE);
	return remove (Tags::TypeUid<T>::get_uid ());
}

template <typename T>
bool
Tags::peek (T *tag) const
{
	assert (sizeof (T) <= Tags::SIZE);
	uint8_t *buf = reinterpret_cast<uint8_t *> (tag);
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		if (cur->m_id == TypeUid<T>::get_uid ()) {
			/* found tag */
			memcpy (buf, cur->m_data, sizeof (T));
			return true;
		}
	}
	/* no tag found */
	return false;
}

template <typename T>
bool
Tags::update (T const*tag)
{
	assert (sizeof (T) <= Tags::SIZE);
	uint8_t const*buf = reinterpret_cast<uint8_t const*> (tag);
	return update (buf, TypeUid<T>::get_uid ());
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
	remove_all ();
	m_next = o.m_next;
	if (m_next != 0) {
		m_next->m_count++;
	}
	return *this;
}

Tags::~Tags ()
{
	remove_all ();
}

void
Tags::remove_all (void)
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
