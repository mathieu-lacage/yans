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
#include <string.h>

namespace yans {

struct GTags::TagData *GTags::m_free = 0;
uint32_t GTags::m_n_free = 0;

GTags::GTags ()
	: m_next ()
{}

GTags::GTags (GTags const &o)
	: m_next (o.m_next)
{
	if (m_next != 0) {
		m_next->m_count++;
	}
}

GTags &
GTags::operator = (GTags const &o)
{
	remove_all_tags ();
	m_next = o.m_next;
	if (m_next != 0) {
		m_next->m_count++;
	}
	return *this;
}

GTags::~GTags ()
{
	remove_all_tags ();
}

void
GTags::remove_all_tags (void)
{
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		cur->m_count--;
		if (cur->m_count > 0) {
			break;
		}
		free_data (cur);
	}
	m_next = 0;
}

struct GTags::TagData *
GTags::alloc_data (void)
{
	struct TagData *retval;
	if (m_free != 0) {
		retval = m_free;
		m_free = retval->m_next;
		m_n_free--;
	} else {
		retval = new struct TagData ();
	}
	return retval;
}

void
GTags::free_data (struct TagData *data)
{
	if (m_n_free > 1000) {
		delete data;
		return;
	}
	m_n_free++;
	data->m_next = m_free;
	m_free = data;
}

void
GTags::add (uint8_t const*buffer, uint32_t size, uint32_t id)
{
	// ensure this id was not yet added
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		assert (cur->m_id != id);
	}
	struct TagData *new_start = alloc_data ();
	new_start->m_count = 1;
	new_start->m_next = 0;
	new_start->m_id = id;
	memcpy (new_start->m_data, buffer, size);
	new_start->m_next = m_next;
	m_next = new_start;
}

bool
GTags::remove (uint32_t id)
{
	bool found = false;
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		if (cur->m_id == id) {
			found = true;
		}
	}
	if (!found) {
		return false;
	}
	struct TagData *start = 0;
	struct TagData **prev_next = &start;
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		if (cur->m_id == id) {
			continue;
		}
		struct TagData *copy = alloc_data ();
		copy->m_id = cur->m_id;
		copy->m_count = 1;
		copy->m_next = 0;
		memcpy (copy->m_data, cur->m_data, GTags::SIZE);
		*prev_next = copy;
		prev_next = &copy->m_next;
	}
	*prev_next = 0;
	remove_all_tags ();
	m_next = start;
	return true;
}
bool
GTags::peek (uint8_t *buffer, uint32_t size, uint32_t id) const
{
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		if (cur->m_id == id) {
			/* found tag */
			memcpy (buffer, cur->m_data, size);
			return true;
		}
	}
	/* no tag found */
	return false;
}

bool
GTags::update (uint8_t const*buffer, uint32_t size, uint32_t id)
{
	if (!remove (id)) {
		return false;
	}
	add (buffer, size, id);
	return true;
}


}; // namespace yans

#ifdef RUN_SELF_TESTS

#include "test.h"
#include <iomanip>

namespace yans {

class GTagsTest : Test {
public:
	GTagsTest ();
	virtual ~GTagsTest ();
	virtual bool run_tests (void);
};

struct my_tag_a {
	uint8_t a;
};
struct my_tag_b {
	uint32_t b;
};
struct my_tag_c {
	uint8_t c [GTags::SIZE];
};
struct my_invalid_tag {
	uint8_t invalid [GTags::SIZE+1];
};


GTagsTest::GTagsTest ()
	: Test ("GTags")
{}
GTagsTest::~GTagsTest ()
{}

bool 
GTagsTest::run_tests (void)
{
	bool ok = true;

	// build initial tag.
	GTags tags;
	struct my_tag_a a;
	a.a = 10;
	tags.add (&a);
	a.a = 0;
	tags.peek (&a);
	if (a.a != 10) {
		ok = false;
	}
	struct my_tag_b b;
	b.b = 0xff;
	tags.add (&b);
	b.b = 0;
	tags.peek (&b);
	if (b.b != 0xff) {
		ok = false;
	}

	// make sure copy contains copy.
	GTags other = tags;
	struct my_tag_a o_a;
	o_a.a = 0;
	other.peek (&o_a);
	if (o_a.a != 10) {
		ok = false;
	}
	struct my_tag_b o_b;
	other.peek (&o_b);
	if (o_b.b != 0xff) {
		ok = false;
	}
	// remove data.
	other.remove (&o_a);
	if (other.peek (&o_a)) {
		ok = false;
	}
	if (!tags.peek (&o_a)) {
		ok = false;
	}
	other.remove (&o_b);
	if (other.peek (&o_b)) {
		ok = false;
	}
	if (!tags.peek (&o_b)) {
		ok = false;
	}

	other = tags;
	GTags another = other;
	struct my_tag_c c;
	c.c[0] = 0x66;
	another.add (&c);
	c.c[0] = 0;
	another.peek (&c);
	if (!another.peek (&c)) {
		ok = false;
	}
	if (tags.peek (&c)) {
		ok = false;
	}



	//struct my_invalid_tag invalid;
	//tags.add (&invalid);

	return ok;
}

static GTagsTest g_tags_test;


}; // namespace yans

#endif /* RUN_SELF_TESTS */

