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
#ifndef MAC_HANDLER_TCC
#define MAC_HANDLER_TCC

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


class MacCancelableEvent : public Event
{
public:
	MacCancelableEvent ();
	bool isCanceled (void);
	void cancel (void);
	double getStartTime (void);
private:
	bool m_canceled;
	double m_startTime;
};

template<typename TYPE>
class DynamicHandler : public Handler
{
public:
	typedef void (TYPE::*DynamicHandler_TYPE)(MacCancelableEvent *event);
	DynamicHandler (TYPE *obj, DynamicHandler_TYPE handler);
	virtual ~DynamicHandler ();
	void start (MacCancelableEvent *e, double delay);
	void start (double delay);
	bool isRunning (void);
	double getEndTime (void);
	void cancel (void);
	virtual void handle (Event *e);
private:
	TYPE *m_obj;
	DynamicHandler_TYPE m_handler;
	MacCancelableEvent *m_event;
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
	/* we need to re-initialize the uid_ to make sure 
	 * the scheduler believes the event is brand new. 
	 * If you do not do this, veeeeery bad things will
	 * happen. Trust me: you don't want to do it.
	 * Ok, you don't want to trust me:
	 *   - if uid_ is not >= 0, the scheduler aborts (assert)
	 *   - if time_ is not re-initialized, and if you try
	 *     to re-schedule this very same StaticHandler from
	 *     its handler (call start from handler), you will
	 *     see a very confused scheduler.
	 * Been there, done that.
	 */
	m_event.uid_ = 0;
	m_event.time_ = 0.0;
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






template <typename TYPE>
DynamicHandler<TYPE>::DynamicHandler<TYPE> (TYPE *obj, DynamicHandler_TYPE handler)
	: m_obj (obj) , m_handler (handler),
	  m_event (0)
{}
template <typename TYPE>
DynamicHandler<TYPE>::~DynamicHandler<TYPE> ()
{}
template <typename TYPE>
void 
DynamicHandler<TYPE>::start (class MacCancelableEvent *e, double delay)
{
	m_event = e;
	Scheduler::instance ().schedule (this, e, delay);
}
template <typename TYPE>
void 
DynamicHandler<TYPE>::start (double delay)
{
	start (new MacCancelableEvent (), delay);
}
template <typename TYPE>
bool 
DynamicHandler<TYPE>::isRunning (void)
{
	if (m_event != 0 &&
	    !m_event->isCanceled ()) {
		return true;
	} else {
		return false;
	}
}
template <typename TYPE>
double
DynamicHandler<TYPE>::getEndTime (void)
{
	if (isRunning ()) {
		return m_event->time_;
	} else {
		return 0.0;
	}
}
template <typename TYPE>
void 
DynamicHandler<TYPE>::cancel (void)
{
	if (isRunning ()) {
		m_event->cancel ();
		m_event = 0;
	}
}
template <typename TYPE>
void 
DynamicHandler<TYPE>::handle (Event *e)
{
	MacCancelableEvent *ev = static_cast<MacCancelableEvent *> (e);
	if (!ev->isCanceled ()) {
		m_event = 0;
		(m_obj->*m_handler) (ev);
	}
	delete ev;
}


#endif /* MAC_HANDLER_TCC */
