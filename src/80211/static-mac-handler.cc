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

#include "static-mac-handler.h"

StaticMacHandler::StaticMacHandler (MacLow80211 *mac, StaticMacRxHandler handler)
	: m_mac (mac) , m_handler (handler)
{}
StaticMacHandler::~StaticMacHandler ()
{}
void 
StaticMacHandler::start (double delay)
{
	// XXX: I am not sure why we need to re-initialize the Event.
	m_event.uid_ = 0;
	Scheduler::instance ().schedule (this, &m_event, delay);
}
double
StaticMacHandler::now (void)
{
	return Scheduler::instance ().clock ();
}
bool
StaticMacHandler::isRunning (void)
{
	if (m_event.time_ > now ()) {
		return true;
	} else {
		return false;
	}
}
void 
StaticMacHandler::handle (Event *e)
{
	(m_mac->*m_handler) ();
}

