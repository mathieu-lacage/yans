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

#include "event-binary-heap.h"

namespace yans {

EventBinaryHeap::EventBinaryHeap ()
{}

EventBinaryHeap::~EventBinaryHeap ()
{}

EventId 
EventBinaryHeap::insert_at_us (Event *event, uint64_t time)
{
	return EventId ();
}

Event   *
EventBinaryHeap::peek_next (void)
{
	return 0;
}
uint64_t 
EventBinaryHeap::peek_next_time_us (void)
{
	// XXX
	return 0;
}
void     
EventBinaryHeap::remove_next (void)
{}

Event *
EventBinaryHeap::remove (EventId id)
{
	// XXX
	return 0;
}

void 
EventBinaryHeap::clear (void)
{}
void 
EventBinaryHeap::print_debug (void)
{}

}; // namespace yans
