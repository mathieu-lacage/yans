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
#ifndef TAG_H
#define TAG_H

#include <stdint.h>
#include <vector>

namespace yans {

class TagManager {
public:
	static uint32_t register_tag (char const *name);
private:
	static TagManager *instance (void);
	uint32_t m_tag_id;
	TagManager ();
};


class Tag {
public:
	virtual ~Tag () = 0;
private:
	virtual uint32_t real_get_id (void) const = 0;
	virtual uint32_t real_get_size (void) const = 0;
	friend class Tags;
};

class Tags {
public:
	Tags ();
	~Tags ();

	void reset (void);

	Tags *copy (void) const;

	void add (Tag const *tag);
	void peek (Tag *tag);
	void remove (Tag *tag);
	void update (Tag *tag);
private:
	struct IndexEntry {
		uint32_t id;
		uint32_t size;
		uint32_t start;
	};
	typedef std::vector<struct IndexEntry> Index;
	typedef std::vector<struct IndexEntry>::iterator IndexI;

	Tags (Tags const&o);
	void reserve_at_end (uint32_t room);
	uint32_t m_real_size;
	uint32_t m_size;
	uint8_t *m_data;
	Index m_index;
};

}; // namespace yans


#endif /* TAG_H */
