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

#include "phy-event-list.h"

#include <list>

#include <float.h>

#define nopeEVENT_TRACE 1

#ifdef EVENT_TRACE
# define TRACE_RECORD(format, ...) \
  printf ("RECORD " format "\n", ## __VA_ARGS__);
# define TRACE_UNRECORD(format, ...) \
  printf ("UNRECORD " format "\n", ## __VA_ARGS__);
#else /* EVENT_TRACE */
# define TRACE_RECORD(format, ...)
# define TRACE_UNRECORD(format, ...)
#endif /* EVENT_TRACE */


/****************************************
 *  Implementation of Event class
 ***************************************/
PhyStateEvent::PhyStateEvent (enum PhyStateEventType type, double time)
	: m_type (type),
	  m_time (time)
{}

enum PhyStateEvent::PhyStateEventType 
PhyStateEvent::getType (void)
{
	return m_type;
}
double 
PhyStateEvent::getTime (void)
{
	return m_time;
}

/****************************************
 *  Implementation of specialized Event 
 *  class
 ***************************************/

PhyStateEventSyncEnd::PhyStateEventSyncEnd (double time, bool wasSuccessful) 
	: PhyStateEvent (PhyStateEvent::SYNC_END, time),
	  m_wasSuccessful (wasSuccessful)
{}
bool
PhyStateEventSyncEnd::getWasSuccessful (void) 
{
	return m_wasSuccessful;
}



/****************************************
 *  Implementation of real EventList 
 *  class
 ***************************************/

Phy80211EventList::Phy80211EventList ()
	: m_backoffReservation (0),
	  m_lastRxEndTime (0.0),
	  m_wasLastRxSuccessful (true),
	  m_lastTxEndTime (0.0)
{}
Phy80211EventList::~Phy80211EventList ()
{}


void 
Phy80211EventList::notifyRxStart (double now)
{
	TRACE_RECORD ("rx start at %f", now);

	PhyStateEvent *event;
	event = new PhyStateEvent (PhyStateEvent::SYNC_START, now);
	m_list.push_back (event);
}
void 
Phy80211EventList::notifyRxEnd (double now, bool receivedOk)
{
	TRACE_RECORD ("rx end at %f, %s", now, ((receivedOk)?"ok":"failed"));
	PhyStateEvent *event;
	event = new PhyStateEventSyncEnd (now, receivedOk);
	m_list.push_back (event);
}
void 
Phy80211EventList::notifyTxStart (double now, double duration)
{
	TRACE_RECORD ("tx start at %f", now);
	TRACE_RECORD ("tx end at %f", now + duration);
	PhyStateEvent *event;
	event = new PhyStateEvent (PhyStateEvent::TX_START, now);
	m_list.push_back (event);
	event = new PhyStateEvent (PhyStateEvent::TX_END, now + duration);
	m_list.push_back (event);
}
void 
Phy80211EventList::notifySleep (double now)
{
	TRACE_RECORD ("sleep at %f", now);
	PhyStateEvent *event;
	event = new PhyStateEvent (PhyStateEvent::SLEEP, now);
	m_list.push_back (event);
}
void 
Phy80211EventList::notifyWakeup (double now)
{
	TRACE_RECORD ("wakeup at %f", now);
	PhyStateEvent *event;
	event = new PhyStateEvent (PhyStateEvent::WAKEUP, now);
	m_list.push_back (event);
}


std::list<PhyStateEvent *>::const_iterator
Phy80211EventList::begin (void)
{
	return m_list.begin ();
}
std::list<PhyStateEvent *>::const_iterator
Phy80211EventList::end (void)
{
	return m_list.end ();
}
int 
Phy80211EventList::size (void)
{
	return m_list.size ();
}

double 
Phy80211EventList::getLastRxEndTime (void)
{
	return m_lastRxEndTime;
}
bool 
Phy80211EventList::wasLastRxEndSuccessful (void)
{
	return m_wasLastRxSuccessful;
}
double 
Phy80211EventList::getLastTxEndTime (void)
{
	return m_lastTxEndTime;
}

int 
Phy80211EventList::notifyBackoffStart (double now)
{
	m_backoffReservation++;
	m_backoffReservations.push_back (std::pair<int, double> (m_backoffReservation, now));
	return m_backoffReservation;
}

void
Phy80211EventList::notifyBackoffEnd (int backoffId)
{
	std::list<std::pair<int, double> >::iterator i;
	i = m_backoffReservations.begin ();
	while (i != m_backoffReservations.end ()) {
		if ((*i).first == backoffId) {
			m_backoffReservations.erase (i);
			break;
		}
		i++;
	}
	clearEventList ();
}

void 
Phy80211EventList::notifyBackoffHandledUntilNow (int backoffId, double now)
{
	std::list<std::pair<int, double> >::iterator i;
	i = m_backoffReservations.begin ();
	while (i != m_backoffReservations.end ()) {
		if ((*i).first == backoffId) {
			(*i).second = now;
			break;
		}
		i++;
	}
	clearEventList ();
}

double 
Phy80211EventList::min (double a, double b)
{
	if (a < b) {
		return a;
	} else {
		return b;
	}
}
void
Phy80211EventList::clearEventList (void)
{
	double oldestBackoffStart;
	std::list<std::pair<int, double> >::iterator i;
	i = m_backoffReservations.begin ();
	oldestBackoffStart = DBL_MAX;
	while (i != m_backoffReservations.end ()) {
		min ((*i).second, oldestBackoffStart);
		i++;
	}
	clearEventListUntil (oldestBackoffStart);
}

void
Phy80211EventList::clearEventListUntil (double until)
{
	/* We remove all the events which are not needed anymore
	 * by the backoff calculation code such that the event list
	 */
	std::list <PhyStateEvent *>::iterator i, first_to_not_remove;
	bool rxing, txing, sleeping;
	i = m_list.begin ();
	first_to_not_remove = m_list.begin ();
	rxing = false;
	txing = false;
	sleeping = false;
	while (i != m_list.end ()) {
		if ((*i)->getTime () > until) {
			break;
		}
		switch ((*i)->getType ()) {
		case PhyStateEvent::SYNC_END:
			rxing = false;
			break;
		case PhyStateEvent::TX_END:
			txing = false;
			break;
		case PhyStateEvent::WAKEUP:
			sleeping = false;
			break;
		case PhyStateEvent::TX_START:
			txing = true;
			break;
		case PhyStateEvent::SYNC_START:
			rxing = true;
			break;
		case PhyStateEvent::SLEEP:
			sleeping = true;
			break;
		}
		i++;
		if (!rxing && !txing && !sleeping) {
			first_to_not_remove = i;
		}
	}

	i = m_list.begin ();
	//int j = 0;
	while (i != m_list.end () &&
	       i != first_to_not_remove) {
		switch ((*i)->getType ()) {
		case PhyStateEvent::SYNC_END: {
			PhyStateEventSyncEnd *event = static_cast<PhyStateEventSyncEnd *> (*i);
			TRACE_UNRECORD ("rx end at %f, %s", event->getTime (), (event->getWasSuccessful ()?"ok":"failed"));
			m_lastRxEndTime = event->getTime ();
			m_wasLastRxSuccessful = event->getWasSuccessful ();
		} break;
		case PhyStateEvent::TX_END:
			TRACE_UNRECORD ("tx end at %f", (*i)->getTime ());
			m_lastTxEndTime = (*i)->getTime ();
			break;
		case PhyStateEvent::TX_START:
			TRACE_UNRECORD ("tx start at %f", (*i)->getTime ());
			break;
		case PhyStateEvent::SYNC_START:
			TRACE_UNRECORD ("rx end at %f", (*i)->getTime ());
			break;
		case PhyStateEvent::SLEEP:
			TRACE_UNRECORD ("sleep at %f", (*i)->getTime ());
			break;
		case PhyStateEvent::WAKEUP:
			TRACE_UNRECORD ("wakeup at %f", (*i)->getTime ());
			break;
		}
		i = m_list.erase (i);
		//j++;
	}
#if 0
	if (j > 0) {
		printf ("removed %d\n", j);
	}
#endif
}
