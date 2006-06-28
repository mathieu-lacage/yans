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

/**
 * \brief allocate Tag ids
 *
 * This is a helper class used to implement
 * every subclass of the Tag class.
 */
class TagManager {
public:
	/**
	 * \param name a string to identify the tag being registered
	 * \return a unique id.
	 *
	 * Every call to this method returns a different unique id.
	 */
	static uint32_t register_tag (char const *name);
private:
	static TagManager *instance (void);
	uint32_t m_tag_id;
	TagManager ();
};


/**
 * \brief per-packet Tags
 *
 * Every user tag should derive from this abstract base class
 * to be able to be added to a Packet. This base class provides
 * functionality to allow the Packet class to uniquely identify
 * the type of each Tag instance.
 * A subclass 
 */
class Tag {
public:
	/**
	 * Derived classes must provide a virtual destructor
	 */
	virtual ~Tag () = 0;
private:
	/**
	 * \return an identifier specific to the Tag type.
	 *
	 * This method is typically implemented by returning
	 * an id allocated with TagManager::register_tag.
	 */
	virtual uint32_t real_get_id (void) const = 0;
	/**
	 * \return the size of the subclass.
	 *
	 * This method is typically implemented by returning
	 * sizeof (*this).
	 */
	virtual uint32_t real_get_size (void) const = 0;
	friend class Tags;
};

class Tags {
public:
	Tags ();
	~Tags ();

	void reset (void);

	void copy_from (Tags const &o);

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
