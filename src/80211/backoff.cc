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

#include <iostream>

#include "backoff.h"
#include "phy-80211.h"
#include "phy-event-list.h"
#include "mac-low-80211.h"

#define nopeEVENT_TRACE 1

#ifdef EVENT_TRACE
# define TRACE(format, ...) \
  printf ("BACKOFF " format "\n", ## __VA_ARGS__);
#else /* EVENT_TRACE */
# define TRACE(format, ...)
#endif /* EVENT_TRACE */


class NavEvent {
public:
	enum NavEventType {
		NAV_EVENT_START,
		NAV_EVENT_END
	};
	NavEvent (enum NavEventType type, double time)
		: m_type (type),
		  m_time (time)
	{}
	void setTime (double time) {
		m_time = time;
	}
	double getTime (void) {
		return m_time;
	}
	enum NavEventType getType (void) {
		return m_type;
	}
private:
	enum NavEventType m_type;
	double m_time;
};

Backoff::Backoff (MacLow80211 *mac)
	: m_mac (mac),
	  m_backoffStart (0.0),
	  m_backoffLeft (0.0),
	  m_NAV_Running (false),
	  m_lastNAV_BackoffStart (0.0),
	  m_lastTxBackoffStart (0.0),
	  m_lastRxBackoffStart (0.0),
	  m_backoffId (0),
	  m_txing (false),
	  m_rxing (false),
	  m_sleeping (false)
{}

Phy80211EventList *
Backoff::getPhyEventList (void)
{
	return m_mac->peekPhy80211 ()->peekEventList ();
}

double
Backoff::getDIFS (void)
{
	// XXX
	return m_mac->getDIFS ();
}
double
Backoff::getEIFS (void)
{
	// XXX
	return m_mac->getEIFS ();
}

double
Backoff::max (double a, double b)
{
	if (a > b) {
		return a;
	} else {
		return b;
	}
}

double
Backoff::mostRecent (double a, double b, double c, double d)
{
	double retval;
	retval = max (a, b);
	retval = max (retval, c);
	retval = max (retval, d);
	return retval;
}

void
Backoff::updateBackoff (double time)
{
	if (!m_NAV_Running &&
	    !m_txing &&
	    !m_rxing &&
	    !m_sleeping) {
		double mostRecentEvent = mostRecent (m_lastRxBackoffStart,
						     m_lastTxBackoffStart,
						     m_lastNAV_BackoffStart,
						     m_backoffStart);
		/* It is possible for mostRecentEvent to be in 
		 * the future because it might take the value
		 * of m_backoffStart.
		 */
		if (mostRecentEvent < time) {
			m_backoffLeft -= time - mostRecentEvent;
			m_backoffLeft = max (m_backoffLeft, 0.0);
			/* backoffStart is never decreasing.
			 * This is very important. _Really_.
			 */
			assert (mostRecentEvent >= m_backoffStart);
			m_backoffStart = mostRecentEvent;
		}
	}
}
void
Backoff::recordTxStart (double time)
{
	TRACE ("tx start at %f", time);
	updateBackoff (time);
	m_txing = false;
}
void
Backoff::recordTxEnd (double time)
{
	TRACE ("tx end at %f", time);
	m_lastTxBackoffStart = time + getDIFS ();
	m_txing = false;
}
void
Backoff::recordSyncStart (double time)
{
	TRACE ("rx start at %f", time);
	updateBackoff (time);
	m_rxing = true;
}
void
Backoff::recordSyncEnd (double time, bool receivedOk)
{
	TRACE ("rx end at %f, %s", time, (receivedOk?"ok":"failed"));
	if (receivedOk) {
		m_lastRxBackoffStart = time + getDIFS ();
	} else {
		m_lastRxBackoffStart = time + getEIFS ();
	}
	m_rxing = false;
}
void
Backoff::recordSleep (double time)
{
	TRACE ("sleep at %f", time);
	assert (!m_txing);
	m_sleeping = true;
	m_rxing = false;
}
void
Backoff::recordWakeup (double time)
{
	TRACE ("wakeup at %f", time);
	m_sleeping = false;
}
void
Backoff::recordNAV_BusyStart (double time)
{
	TRACE ("nav busy start at %f", time);
	updateBackoff (time);
	m_NAV_Running = true;
}
void
Backoff::recordNAV_BusyEnd (double time)
{
	TRACE ("nav busy end at %f", time);
	m_lastNAV_BackoffStart = time;
	m_NAV_Running = false;
}


void
Backoff::clearNAV_Until (double time)
{
	std::list<NavEvent *>::iterator i, start, end;
	for (i = m_navList.begin (); i != m_navList.end (); ) {
		start = i;
		i++;
		end = i;
		i++;
		assert (start != 0);
		assert (end != 0);
		assert ((*start)->getType () == NavEvent::NAV_EVENT_START);
		assert ((*end)->getType () == NavEvent::NAV_EVENT_END);
		assert ((*start)->getTime () <= (*end)->getTime ());
		if ((*end)->getTime () > time) {
			break;
		}
		//std::cout << "remove "<< (*start)->getTime ()<< std::endl;
		delete (*start);
		delete (*end);
		m_navList.erase (start);
		m_navList.erase (end);
	}
}



/******************************************
 *  The public API starts below.
 ******************************************/

void
Backoff::updateToNow (double now)
{
	TRACE ("update to %f", now);
	/* Initialize our state with the information for the previously
	 * received and transmitted packets which are not present in
	 * the event list. The event list always starts with an IDLE
	 * state.
	 */
	m_txing = false;
	m_rxing = false;
	m_sleeping = false;
	recordSyncEnd (getPhyEventList ()->getLastRxEndTime (),
		       getPhyEventList ()->wasLastRxEndSuccessful ());
	recordTxEnd (getPhyEventList ()->getLastTxEndTime ());

	/* We merge the two lists of events and invoke the 
	 * right event handling methods for each event.
	 */
	std::list<PhyStateEvent *>::const_iterator i;
	std::list<NavEvent *>::const_iterator j;
	//cout << "nav list: " << m_navList.size () << " event list: " << getPhyEventList ()->size () << endl;
	j = m_navList.begin ();
	for (i = getPhyEventList ()->begin (); i != getPhyEventList ()->end (); i++) {
		while (true) {
			if (j == m_navList.end ()) {
				break;
			}
			if ((*j)->getTime () > (*i)->getTime ()) {
				break;
			} else {
				if ((*j)->getType () == NavEvent::NAV_EVENT_START) {
					recordNAV_BusyStart ((*j)->getTime ());
				} else {
					recordNAV_BusyEnd ((*j)->getTime ());
				}
				j++;
			}
		}
		switch ((*i)->getType ()) {
		case PhyStateEvent::TX_START:
			recordTxStart ((*i)->getTime ());
			break;
		case PhyStateEvent::TX_END:
			recordTxEnd ((*i)->getTime ());
			break;
		case PhyStateEvent::SYNC_START:
			recordSyncStart ((*i)->getTime ());
			break;
		case PhyStateEvent::SYNC_END: {
			PhyStateEventSyncEnd *ev = static_cast <PhyStateEventSyncEnd *> (*i);
			recordSyncEnd ((*i)->getTime (), ev->getWasSuccessful ());
		} break;
		case PhyStateEvent::SLEEP:
			recordSleep ((*i)->getTime ());
			break;
		case PhyStateEvent::WAKEUP:
			recordWakeup ((*i)->getTime ());
			break;
		}
	}
	/* finish handling the events in the navList if there
	 * is anything left in it. 
	 */
	while (j != m_navList.end () &&
	       (*j)->getTime () < now) {
		if ((*j)->getType () == NavEvent::NAV_EVENT_START) {
			recordNAV_BusyStart ((*j)->getTime ());
		} else {
			recordNAV_BusyEnd ((*j)->getTime ());
		}
		j++;
	}

	updateBackoff (now);

	/* Kill all NAVs which were completed before now. 
	 * Leave in any running NAV.
	 */
	clearNAV_Until (now);
	assert (m_navList.empty () ||
		m_navList.size () == 2);

	/* Notify the phy event list that we have handled all of 
	 * these events. The PHY event list will try to get rid of
	 * them if no other MAC has started a backoff which 
	 * includes these events.
	 */
	getPhyEventList ()->notifyBackoffHandledUntilNow (m_backoffId, now);
}

void
Backoff::updateNAV (double duration, double now)
{
	if (duration == 0.0) {
		// XXX is this okay ?
		return;
	}
	/* We have received a packet which contains a 
	 * valid duration field. As such, we need to
	 * update the NAV now if it is sufficiently new.
	 */
	if (isVirtualCS_Idle (now)) {
		/* we start a new NAV BUSY period 
		 * now since the previous NAV BUSY
		 * period is finished.
		 */
		NavEvent *nav;
		nav = new NavEvent (NavEvent::NAV_EVENT_START, now);
		m_navList.push_back (nav);
		nav = new NavEvent (NavEvent::NAV_EVENT_END, now + duration);
		m_navList.push_back (nav);
	} else {
		/* the previous NAV BUSY period
		 * is not finished so we want to
		 * see if we need to update it with
		 * this duration.
		 */
		assert (nav->getType () == NavEvent::NAV_EVENT_END);
		NavEvent *nav = m_navList.back ();
		if (now + duration > nav->getTime ()) {
			nav->setTime (now + duration);
		} else {
			/* this packet does not update 
			 * the NAV.
			 */
		}
	} 
}

void
Backoff::cancel (void)
{
	m_backoffLeft = 0.0;
	getPhyEventList ()->notifyBackoffEnd (m_backoffId);
}
void
Backoff::start (double backoffStart, double backoffDuration)
{
	assert (m_backoffStart <= backoffStart);
	m_backoffStart = backoffStart;
	m_backoffLeft = backoffDuration;
	m_backoffId = getPhyEventList ()->notifyBackoffStart (backoffStart);
}

bool
Backoff::isCompleted (void) const
{
	if (m_backoffLeft <= 0.0) {
		return true;
	} else {
		return false;
	}
}

double
Backoff::getDurationLeft (void) const
{
	return m_backoffLeft;
}

double 
Backoff::getExpectedDelayToEndFromNow (double now)
{
	updateToNow (now);
	return getDurationLeft ();
#if 0
	// XXX We need something really smart to do better than getDurationLeft.
	if (m_backoffLeft <= 0.0) {
		return 0.0;
	} else {
		assert (m_backoffStart >= now);
		cout << "backoff start " << m_backoffStart << " now " << now << " left " << m_backoffLeft << endl;
		return m_backoffStart - now + m_backoffLeft;
	}
#endif
}

bool
Backoff::isVirtualCS_Idle (double now)
{
	if (m_navList.empty ()) {
		return true;
	}
	NavEvent *nav = m_navList.back ();
	if (nav->getTime () < now) {
		return true;
	} else {
		return false;
	}
}
double 
Backoff::getDelayUntilIdle (double now)
{
	if (m_navList.empty ()) {
		return 0.0;
	}
	NavEvent *nav = m_navList.back ();
	double delay = nav->getTime () - now;
	return delay;
}
