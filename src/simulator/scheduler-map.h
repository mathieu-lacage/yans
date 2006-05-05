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

#ifndef SCHEDULER_MAP_H
#define SCHEDULER_MAP_H

#include "scheduler.h"
#include <stdint.h>
#include <map>
#include <utility>

namespace yans {

class SchedulerMap : public Scheduler {
public:
	SchedulerMap ();
	virtual ~SchedulerMap ();

	virtual Event *insert_at_us (Event *event, uint64_t time);

	virtual Event   *peek_next (void);
	virtual uint64_t peek_next_time_us (void);
	virtual void     remove_next (void);

	virtual Event *remove (Event const*ev);

	virtual void clear (void);
	virtual void print_debug (void);
private:
	/* Note that we have to use a key made of the expiration time
	 * and a uid unique to each packet because std::map is a unique
	 * key sorted container. i.e., no two elements can have "equal"
	 * keys.
	 */
	struct EventMapKey {
		uint64_t m_time;
		uint32_t m_uid;
	};
	class EventMapKeyCompare {
	public:
		bool operator () (struct EventMapKey a, struct EventMapKey b);
	};
	typedef std::map<EventMapKey, Event*, EventMapKeyCompare> EventMap;
	typedef std::map<EventMapKey, Event*, EventMapKeyCompare>::iterator EventMapI;

	void store_in_event (Event *ev, EventMapI i) const;
	EventMapI get_from_event (Event const *ev) const;


	EventMap m_list;
	uint32_t m_uid;
};

}; // namespace yans


#endif /* SCHEDULER_MAP_H */
