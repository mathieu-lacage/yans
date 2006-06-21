/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#ifndef EVENT_IMPL_H
#define EVENT_IMPL_H

#include <stdint.h>

namespace yans {

class EventImpl {
public:
	INL_EXPE EventImpl ();
	INL_EXPE void ref (void);
	INL_EXPE void unref (void);
	INL_EXPE void invoke (void);
	INL_EXPE void set_tag (void *tag);
	INL_EXPE void *get_tag (void) const;
	INL_EXPE void cancel (void);
	INL_EXPE bool is_running (void);
protected:
	virtual ~EventImpl () = 0;
private:
	virtual void notify (void) = 0;
private:
	void *m_id;
	uint32_t m_ref_count;
	uint32_t m_cancel : 1;
	uint32_t m_running : 1;
};

}; // namespace yans

#ifdef INL_EXPE
namespace yans {
EventImpl::EventImpl ()
	: m_id (0),
	  m_ref_count (1),
	  m_cancel (0),
	  m_running (1)
{}

void
EventImpl::ref (void)
{
	m_ref_count++;
}

void
EventImpl::unref (void)
{
	m_ref_count--;
	if (m_ref_count == 0) {
		delete this;
	}
}

void 
EventImpl::invoke (void)
{
	if (m_cancel == 0) {
		notify ();
	}
	m_running = 0;
}
void 
EventImpl::set_tag (void *tag)
{
	m_id = tag;
}
void *
EventImpl::get_tag (void) const
{
	return m_id;
}
void
EventImpl::cancel (void)
{
	m_cancel = 1;
	m_running = 0;
}
bool 
EventImpl::is_running (void)
{
	return (m_running == 1);
}
}; // namespace yans
#endif


#endif /* EVENT_IMPL_H */
