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

#ifndef EVENT_H
#define EVENT_H

#include <algorithm>

namespace yans {


class EventImpl;
/**
 * \brief Simulation events.
 *
 * The Event class has POD semantics: it can and should
 * be passed around by value. The Event class is a mere
 * wrapper around the EventImpl class which performs
 * memory management of EventImpl object instances.
 *
 * While users could create Events by instanciating 
 * subclasses of the EventImpl class and storing them
 * in an Event instance, they are advised to use the 
 * template functions \ref make_event instead.
 */
class Event {
public:
	INL_EXPE Event ();
	INL_EXPE Event (EventImpl *impl);
	INL_EXPE Event (Event const &o);
	INL_EXPE ~Event ();
	INL_EXPE Event &operator = (Event const&o);
	INL_EXPE void operator () (void);
	/**
	 * Cancel an event. This operation has O(1) 
	 * complexity since it merely sets a "cancel" bit
	 * to on and does not remove the Event from the 
	 * scheduler's event list. When the event expires, 
	 * the scheduler checks this cancel bit and, if set,
	 * does not execute the event.
	 */
	INL_EXPE void cancel (void);
	/**
	 * Return true if the event is in RUNNING state.
	 * Return false otherwise.
	 *
	 * An Event is created in RUNNING state and switches
	 * to NON_RUNNING state upon one of:
	 *    - cancel bit is set to on
	 *    - Event execution is completed.
	 * It is important to note that an event is in RUNNING
	 * state while being executed.
	 */
	INL_EXPE bool is_running (void);
private:
	friend class SchedulerHeap;
	friend class SchedulerList;
	friend class SchedulerMap;
	friend INL_EXPE void swap (Event &a, Event &b);
	INL_EXPE void set_tag (void *tag);
	INL_EXPE void *get_tag (void) const;
	EventImpl *m_impl;
};

INL_EXPE void swap (Event &a, Event &b);


}; // namespace yans

#ifdef INL_EXPE
#include "event-impl.h"
#include <cassert>

namespace yans {

Event::Event ()
	: m_impl (0)
{}
Event::Event (EventImpl *impl)
	: m_impl (impl)
{}
Event::Event (Event const &o)
	: m_impl (o.m_impl)
{
	if (m_impl != 0) {
		m_impl->ref ();
	}
}
Event::~Event ()
{
	if (m_impl != 0) {
		m_impl->unref ();
	}
	m_impl = 0;
}
Event &
Event::operator = (Event const&o)
{
	if (m_impl != 0) {
		m_impl->unref ();
	}
	m_impl = o.m_impl;
	if (m_impl != 0) {
		m_impl->ref ();
	}
	return *this;
}
void 
Event::operator () (void)
{
	assert (m_impl != 0);
	m_impl->invoke ();
}
void 
Event::set_tag (void *tag)
{
	m_impl->set_tag (tag);
}
void *
Event::get_tag (void) const
{
	return m_impl->get_tag ();
}
void
Event::cancel (void)
{
	if (m_impl != 0) {
		m_impl->cancel ();
	}
}

bool 
Event::is_running (void)
{
	if (m_impl != 0 && m_impl->is_running ()) {
		return true;
	} else {
		return false;
	}
}



INL_EXPE void swap (Event &a, Event &b)
{
	std::swap (a.m_impl, b.m_impl);
}

}; // namespace yans
#endif
#endif /* EVENT_H */
