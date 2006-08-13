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
#include "gtags.h"
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
	if (m_next != 0) {
		m_next->m_count--;
		if (m_next->m_count == 0) {
			// XXX
		}
	}
	m_next = o.m_next;
	if (m_next != 0) {
		m_next->m_count++;
	}
	return *this;
}

GTags::~GTags ()
{
	if (m_next != 0) {
		m_next->m_count--;
		if (m_next == 0) {
			// XXX
		}
	}
}

struct GTags::TagData *
GTags::alloc_data (void)
{
	struct TagData *retval;
	if (m_free != 0) {
		retval = m_free;
		m_free = retval->m_next;
		retval->m_next = 0;
		retval->m_count = 1;
		m_n_free--;
	} else {
		retval = new struct TagData ();
		retval->m_count = 1;
		retval->m_next = 0;
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
GTags::add (uint8_t *buffer, uint32_t size, uint32_t id)
{
	// ensure this id was not yet added
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		assert (cur->m_id != id);
	}
	struct TagData *new_start = alloc_data ();
	new_start->m_id = id;
	memcpy (new_start->m_data, buffer, size);
	new_start->m_next = m_next;
	m_next = new_start;
}

void 
GTags::remove (uint32_t id)
{
	struct TagData *new_start = 0;
	struct TagData **prev_next = &new_start;
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		struct TagData *data;
		if (cur->m_count > 1 && 
		    cur->m_id != id) {
			/* we are not the sole owner of this tag so we need to 
			   copy it before updating the next pointer. */
			data = alloc_data ();
			*data = *cur;
			memcpy (data->m_data, cur->m_data, GTags::SIZE);
		} else {
			/* sole owner */
			data = cur;
		}
		if (cur->m_id == id) {
			/* found tag */
			*prev_next = data->m_next;
			m_next = new_start;
			return;
		} else {
			*prev_next = data;
			prev_next = &data->m_next;
		}

	}
	/* no tag found */
	assert (false);
}

void 
GTags::peek (uint8_t *buffer, uint32_t size, uint32_t id)
{
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		if (cur->m_id == id) {
			/* found tag */
			memcpy (buffer, cur->m_data, size);
			return;
		}
	}
	/* no tag found */
	assert (false);
}

void 
GTags::update (uint8_t *buffer, uint32_t size, uint32_t id)
{
	remove (id);
	add (buffer, size, id);
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

	GTags tags;
	struct my_tag_a a;
	struct my_tag_b b;
	struct my_invalid_tag invalid;
	tags.add (&a);
	tags.add (&b);
	tags.add (&invalid);

	GTags other = tags;
	return ok;
}


}; // namespace yans

#endif /* RUN_SELF_TESTS */

