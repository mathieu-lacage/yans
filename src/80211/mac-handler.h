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
#ifndef MAC_HANDLER
#define MAC_HANDLER

#include "mac-low.h"
#include "scheduler.h"

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

class DynamicMacHandler : public Handler
{
public:
	typedef void (MacLow::*DynamicMacRxHandler)(class MacCancelableEvent *event);
	DynamicMacHandler (class MacLow *mac, DynamicMacRxHandler handler);
	virtual ~DynamicMacHandler ();
	void start (class MacCancelableEvent *e, double delay);
	bool isRunning (void);
	double getEndTime (void);
	void cancel (void);
	virtual void handle (Event *e);
private:
	MacLow *m_mac;
	DynamicMacRxHandler m_handler;
	MacCancelableEvent *m_event;
	double m_end;
};




#endif /* MAC_HANDLER */
