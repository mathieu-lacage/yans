/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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

#ifndef TAG_MANAGER_H
#define TAG_MANAGER_H

#include <stdint.h>
#include "sgi-hashmap.h"

namespace yans {

class Tag {};

class TagManager {
public:
	static TagManager *instance (void);

	uint32_t register_tag (char const *name);

	char const *lookup_tag (uint32_t tag_id);

	/* returns zero on failure.
	 */
	uint32_t lookup_tag_id (char const *name);

private:
	void destroy (void);
	typedef Sgi::hash_map<uint32_t, char const *> TagMapping;
	uint32_t m_tag_id;
	TagMapping m_tag_mapping;
	static TagManager *m_instance;
	TagManager ();
};

}; // namespace yans


#endif /* TAG_MANAGER */
