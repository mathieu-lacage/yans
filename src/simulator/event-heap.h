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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#ifndef EVENT_HEAP_H
#define EVENT_HEAP_H

#include <list>
#include <utility>

class Event;

class EventHeap {
 public:
	EventHeap ();
	~EventHeap ();

	/* the insert operations might be veeery slow
	 * but peek_next and remove_next should be
	 * really fast.
	 */
	void insert_at_us (Event *event, uint64_t time);
	void insert_at_s (Event *event, double time);

	Event   *peek_next (void);
	uint64_t peek_next_time_us (void);
	void     remove_next (void);

	void clear (void);

 private:
	typedef std::list<std::pair<Event *, uint64_t> > Events;
	typedef std::list<std::pair<Event *, uint64_t> >::iterator EventsI;
	Events m_events;
};


#endif /* EVENT_HEAP_H */
