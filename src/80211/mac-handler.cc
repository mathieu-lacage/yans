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

#include "mac-handler.h"

MacCancelableEvent::MacCancelableEvent ()
	: m_canceled (false),
	  m_startTime (Scheduler::instance ().clock ())
{}
bool 
MacCancelableEvent::isCanceled (void) 
{
	return m_canceled;
}
void 
MacCancelableEvent::cancel (void)
{
	m_canceled = true;
}
double
MacCancelableEvent::getStartTime (void)
{
	return m_startTime;
}

DynamicMacHandler::DynamicMacHandler (class MacLow *mac, DynamicMacRxHandler handler)
	: m_mac (mac) , m_handler (handler),
	  m_event (0), m_end (0.0)
{}
DynamicMacHandler::~DynamicMacHandler ()
{}
void 
DynamicMacHandler::start (class MacCancelableEvent *e, double delay)
{
	m_event = e;
	m_end = Scheduler::instance ().clock () + delay;
	Scheduler::instance ().schedule (this, e, delay);
}
bool 
DynamicMacHandler::isRunning (void)
{
	if (m_event != 0 &&
	    !m_event->isCanceled ()) {
		return true;
	} else {
		return false;
	}
}
double
DynamicMacHandler::getEndTime (void)
{
	if (isRunning ()) {
		return m_end;
	} else {
		return 0.0;
	}
}
void 
DynamicMacHandler::cancel (void)
{
	if (isRunning ()) {
		m_event->cancel ();
		m_event = 0;
	}
}

void 
DynamicMacHandler::handle (Event *e)
{
	MacCancelableEvent *ev = static_cast<MacCancelableEvent *> (e);
	if (!ev->isCanceled ()) {
		m_event = 0;
		(m_mac->*m_handler) (ev);
	}
	delete ev;
}

