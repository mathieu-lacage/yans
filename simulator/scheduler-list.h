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

#ifndef SCHEDULER_LIST_H
#define SCHEDULER_LIST_H

#include "scheduler.h"
#include <list>
#include <utility>
#include <stdint.h>

namespace yans {

class Event;

class SchedulerList : public Scheduler {
 public:
	SchedulerList ();
	virtual ~SchedulerList ();

	/* the insert operations might be veeery slow
	 * but peek_next and remove_next should be
	 * really fast.
	 */
	virtual EventId insert_at_us (Event *event, uint64_t time);

	virtual Event   *peek_next (void);
	virtual uint64_t peek_next_time_us (void);
	virtual void     remove_next (void);

	virtual Event *remove (EventId id);

	virtual void clear (void);
	virtual void print_debug (void);

	typedef std::list<std::pair<Event *, uint64_t> >::iterator EventsI;
 private:
	typedef std::list<std::pair<Event *, uint64_t> > Events;
	Events m_events;
};

}; // namespace yans


#endif /* SCHEDULER_LIST_H */
