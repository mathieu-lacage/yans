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
	Tags ();
	Tags (Tags const &o);
	Tags &operator = (Tags const &o);
	~Tags ();

	template <typename T>
	void add (T const*tag);

	template <typename T>
	bool remove (T *tag);

	template <typename T>
	bool peek (T *tag) const;

	template <typename T>
	bool update (T const*tag);

	void remove_all_tags (void);

	enum {
		SIZE = 16
	};
private:
	void add (uint8_t const*buffer, uint32_t size, uint32_t id);
	bool remove (uint32_t id);
	bool peek (uint8_t *buffer, uint32_t size, uint32_t id) const;
	bool update (uint8_t const*buffer, uint32_t size, uint32_t id);
	struct TagData *alloc_data (void);
	void free_data (struct TagData *data);

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

}; // namespace yans

#endif /* TAGS_H */
