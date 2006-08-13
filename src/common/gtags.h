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
#ifndef GTAGS_H
#define GTAGS_H

#include <stdint.h>

namespace yans {

class GTags {
public:
	GTags ();
	GTags (GTags const &o);
	GTags &operator = (GTags const &o);
	~GTags ();

	template <typename T>
	void add (T *tag);

	template <typename T>
	bool remove (T *tag);

	template <typename T>
	bool peek (T *tag);

	template <typename T>
	bool update (T *tag);

	enum {
		SIZE = 16
	};
private:
	struct TagData {
		struct TagData *m_next;
		uint32_t m_id;
		uint32_t m_count;
		uint8_t m_data[GTags::SIZE];
	};
	void add (uint8_t *buffer, uint32_t size, uint32_t id);
	bool remove (uint32_t id);
	bool peek (uint8_t *buffer, uint32_t size, uint32_t id);
	bool update (uint8_t *buffer, uint32_t size, uint32_t id);
	struct TagData *alloc_data (void);
	void free_data (struct TagData *data);

	static struct TagData *m_free;
	static uint32_t m_n_free;
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
	union {
		T real_type;
		uint8_t padding[GTags::SIZE];
	};
};
template <typename T>
uint32_t TypeUid<T>::get_uid (void)
{
	static uint32_t uid = UidFactory::create ();
	return uid;
}


template <typename T>
void 
GTags::add (T *tag)
{
	assert (sizeof (T) < GTags::SIZE);
	uint8_t *buf = reinterpret_cast<uint8_t *> (tag);
	add (buf, sizeof (T), TypeUid<T>::get_uid ());
}

template <typename T>
bool
GTags::remove (T *tag)
{
	assert (sizeof (T) < GTags::SIZE);
	uint8_t *buf = reinterpret_cast<uint8_t *> (tag);
	return remove (TypeUid<T>::get_uid ());
}

template <typename T>
bool
GTags::peek (T *tag)
{
	assert (sizeof (T) < GTags::SIZE);
	uint8_t *buf = reinterpret_cast<uint8_t *> (tag);
	return peek (buf, sizeof (T), TypeUid<T>::get_uid ());
}

template <typename T>
bool
GTags::update (T *tag)
{
	assert (sizeof (T) < GTags::SIZE);
	uint8_t *buf = reinterpret_cast<uint8_t *> (tag);
	return update (buf, sizeof (T), TypeUid<T>::get_uid ());
}

}; // namespace yans

#endif /* GTAGS_H */
