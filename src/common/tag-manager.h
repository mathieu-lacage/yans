/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef TAG_MANAGER_H
#define TAG_MANAGER_H

#include <stdint.h>
#include "sgi-hashmap.h"

class Tag {};

class TagManager {
public:
	static TagManager *instance (void);

	void destroy (void);

	uint32_t register_tag (char const *name);

	char const *lookup_tag (uint32_t tag_id);

	/* returns zero on failure.
	 */
	uint32_t lookup_tag_id (char const *name);

private:
	typedef Sgi::hash_map<uint32_t, char const *> TagMapping;
	uint32_t m_tag_id;
	TagMapping m_tag_mapping;
	static TagManager *m_instance;
	TagManager ();
};



#endif /* TAG_MANAGER */
