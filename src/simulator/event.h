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

class Event {
public:
	INL_EXPE Event ();
	INL_EXPE Event (EventImpl *impl);
	INL_EXPE Event (Event const &o);
	INL_EXPE ~Event ();
	INL_EXPE Event &operator = (Event const&o);
	INL_EXPE void operator () (void);
	INL_EXPE void cancel (void);
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
