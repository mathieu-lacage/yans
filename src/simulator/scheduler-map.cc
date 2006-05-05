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
 * The idea to use a std c++ map came from GTNetS
 */

#include "scheduler-map.h"
#include "event.h"
#include <cassert>

#define noTRACE_MAP 1

#ifdef TRACE_MAP
#include <iostream>
# define TRACE(x) \
std::cout << "MAP TRACE " << x << std::endl;
#else /* TRACE_MAP */
# define TRACE(format,...)
#endif /* TRACE_MAP */


namespace yans {

/* Note the invariants which this function must provide:
 * - irreflexibility: f (x,x) is false)
 * - antisymmetry: f(x,y) = !f(y,x)
 * - transitivity: f(x,y) and f(y,z) => f(x,z)
 */
bool
SchedulerMap::EventMapKeyCompare::operator () (struct EventMapKey a, struct EventMapKey b)
{
	assert (a.m_uid != b.m_uid);
	if (a.m_time < b.m_time) {
		return true;
	} else if (a.m_time == b.m_time && a.m_uid < b.m_uid) {
		return true;
	} else {
		return false;
	}
}


SchedulerMap::SchedulerMap ()
	: m_uid (0)
{}
SchedulerMap::~SchedulerMap ()
{}


void 
SchedulerMap::store_in_event (Event *ev, EventMapI i) const
{
	memcpy (&(ev->m_id), &i, sizeof (ev->m_id));
}
SchedulerMap::EventMapI
SchedulerMap::get_from_event (Event const *ev) const
{
	EventMapI i;
	memcpy (&i, &(ev->m_id), sizeof (i));
 	return i;
}

Event * 
SchedulerMap::insert_at_us (Event *event, uint64_t time)
{
	EventMapKey key = {time, m_uid};
	std::pair<EventMapI,bool> result = m_list.insert (std::make_pair (key, event));
	assert (result.second);
	store_in_event (event, result.first);
	m_uid++;
	return event;
}

Event   *
SchedulerMap::peek_next (void)
{
	if (m_list.empty ()) {
		return 0;
	}
	EventMapI i = m_list.begin ();
	assert (i != m_list.end ());
	return (*i).second;
}
uint64_t 
SchedulerMap::peek_next_time_us (void)
{
	if (m_list.empty ()) {
		return 0;
	}
	EventMapI i = m_list.begin ();
	assert (i != m_list.end ());
	return (*i).first.m_time;
}
void     
SchedulerMap::remove_next (void)
{
	assert (!m_list.empty ());
	m_list.erase (m_list.begin ());
}

Event *
SchedulerMap::remove (Event const*ev)
{
	assert (!m_list.empty ());
	return const_cast <Event *> (ev);
}

void 
SchedulerMap::clear (void)
{}
void 
SchedulerMap::print_debug (void)
{}

}; // namespace yans
