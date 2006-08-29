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

#include "yans/stdint.h"
#include <ostream>
#include <vector>

namespace yans {

template <typename T>
class TagPrettyPrinter;

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

	void pretty_print (std::ostream &os);

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

/**
 * This class is used to register a pretty-printer
 * callback function to print in a nice user-friendly
 * way the content of the target type. To register
 * such a type, all you need to do is instantiate
 * an instance of this type any number of times (at
 * least once). Typical users will create static global
 * variable of this type and construct it with
 * the proper function pointer.
 */
template <typename T>
class TagPrettyPrinter {
public:
	TagPrettyPrinter<T> (void(*) (T *, std::ostream &));
private:
	TagPrettyPrinter<T> ();
	static void pretty_print_cb (uint8_t *buf, std::ostream &os);
	static void(*g_pretty_printer) (T *, std::ostream &);
};

class TagsPrettyPrinterRegistry {
public:
	static void record (uint32_t uid, void (*cb) (uint8_t buf[Tags::SIZE], std::ostream &os));
	static void pretty_print (uint32_t uid, uint8_t buf[Tags::SIZE], std::ostream &os);
private:
	typedef std::vector<std::pair<uint32_t, void (*) (uint8_t [Tags::SIZE], std::ostream &)> > PrettyPrinters;
	typedef std::vector<std::pair<uint32_t, void (*) (uint8_t [Tags::SIZE], std::ostream &)> >::iterator PrettyPrintersI;
	static PrettyPrinters g_pretty_printers;
};


}; // namespace yans


#include <cassert>
#include <string.h>

namespace yans {

/**
 * The TypeUid class is used to create a mapping Type --> uid
 * Of course, this class is not perfect: the value of the uid
 * associated to a given type could change accross multiple 
 * runs of the same program on the same platform or accross
 * multiple platforms. There exist no generic portable
 * workaround/solution to this problem also known as 
 * "type id management". The only other reliable solution 
 * is to ask programmers to assign themselves a uid to each 
 * type but this is painful from a management perspective.
 *
 * So, for now, this class is good enough provided you do
 * not try to serialize to permanent storage the type uids
 * generated by this class. Just don't try to do it. It might
 * seem to work but it will fail spectacularily in certain
 * use-cases and you will cry from debugging this. Hear me ?
 */
template <typename T>
class TypeUid {
public:
	static const uint32_t get_uid (void);
private:
	T real_type;
};

template <typename T>
const uint32_t TypeUid<T>::get_uid (void)
{
	static const uint32_t uid = Tags::UidFactory::create ();
	return uid;
}



/**
 * Implementation of the TagPrettyPrinter registration class.
 * It records a callback with the TagPrettyPrinterRegistry.
 * This callback performs type conversion before forwarding
 * the call to the user-provided function.
 */
template <typename T>
TagPrettyPrinter<T>::TagPrettyPrinter (void(*pretty_printer) (T *, std::ostream &))
{
	g_pretty_printer  = pretty_printer;
	TagsPrettyPrinterRegistry::record (TypeUid<T>::get_uid (),
					  &TagPrettyPrinter<T>::pretty_print_cb);
}
template <typename T>
void 
TagPrettyPrinter<T>::pretty_print_cb (uint8_t buf[Tags::SIZE], std::ostream &os)
{
	assert (sizeof (T) <= Tags::SIZE);
	T *tag = reinterpret_cast<T *> (buf);
	(*g_pretty_printer) (tag, os);
}

template <typename T>
void (*TagPrettyPrinter<T>::g_pretty_printer) (T *, std::ostream &) = 0;




template <typename T>
void 
Tags::add (T const*tag)
{
	assert (sizeof (T) <= Tags::SIZE);
	uint8_t const*buf = reinterpret_cast<uint8_t const*> (tag);
	// ensure this id was not yet added
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		assert (cur->m_id != TypeUid<T>::get_uid ());
	}
	struct TagData *new_start = alloc_data ();
	new_start->m_count = 1;
	new_start->m_next = 0;
	new_start->m_id = TypeUid<T>::get_uid ();
	memcpy (new_start->m_data, buf, sizeof (T));
	new_start->m_next = m_next;
	m_next = new_start;
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
