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
#ifndef STATIC_HANDLER_TCC
#define STATIC_HANDLER_TCC

#include "scheduler.h"


template<typename TYPE>
class StaticHandler : public Handler
{
public:
	typedef void (TYPE::*StaticHandler_TYPE) (void);
	StaticHandler (TYPE *obj, StaticHandler_TYPE handler);
	virtual ~StaticHandler ();
	void start (double delay);
	virtual void handle (Event *e);
	bool isRunning (void);
private:
	double now (void);
	TYPE *m_obj;
	StaticHandler_TYPE m_handler;
	Event m_event;
};


template <typename TYPE>
StaticHandler<TYPE>::StaticHandler<TYPE> (TYPE *obj, StaticHandler_TYPE handler)
	: m_obj (obj) , m_handler (handler)
{}
template <typename TYPE>
StaticHandler<TYPE>::~StaticHandler<TYPE> ()
{}
template <typename TYPE>
void 
StaticHandler<TYPE>::start (double delay)
{
	// XXX: I am not sure why we need to re-initialize the Event.
	m_event.uid_ = 0;
	Scheduler::instance ().schedule (this, &m_event, delay);
}
template <typename TYPE>
double
StaticHandler<TYPE>::now (void)
{
	return Scheduler::instance ().clock ();
}
template <typename TYPE>
bool
StaticHandler<TYPE>::isRunning (void)
{
	if (m_event.time_ > now ()) {
		return true;
	} else {
		return false;
	}
}
template <typename TYPE>
void 
StaticHandler<TYPE>::handle (Event *e)
{
	(m_obj->*m_handler) ();
}

#endif /* STATIC_HANDLER_TCC */
