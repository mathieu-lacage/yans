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
#include "mac-low-80211.h"
#include "precision.h"

#define nopeBACKOFF_TRACE 1

#ifdef BACKOFF_TRACE
# define TRACE(format, ...) \
  printf ("BACKOFF %d " format "\n", m_mac->getSelf (), ## __VA_ARGS__);
#else /* BACKOFF_TRACE */
# define TRACE(format, ...)
#endif /* BACKOFF_TRACE */



Backoff::Backoff (MacLow *mac)
	: m_mac (mac),
	  m_backoffStart (0.0),
	  m_backoffLeft (0.0),
	  m_lastNavStart (0.0),
	  m_lastNavDuration (0.0),
	  m_lastRxStart (0.0),
	  m_lastRxDuration (0.0),
	  m_lastRxReceivedOk (true),
	  m_lastRxEnd (0.0),
	  m_lastTxStart (0.0),
	  m_lastTxDuration (0.0),
	  m_lastSleepStart (0.0),
	  m_lastWakeupStart (0.0),
	  m_rxing (false),
	  m_sleeping (false)
{}

Backoff::~Backoff ()
{}

double 
Backoff::getLastRxEndTime (void) const
{
	return m_lastRxEnd;
}
double 
Backoff::getLastTxEndTime (void) const
{
	return m_lastTxStart + m_lastTxDuration;
}
bool 
Backoff::wasLastRxOk (void) const
{
	return m_lastRxReceivedOk;
}
bool
Backoff::isCompleted (double now)
{
	updateBackoff (now);
	if (m_backoffLeft <= 0.0) {
		return true;
	} else {
		return false;
	}
}

double
Backoff::getDelayUntilEnd (double now)
{
	updateBackoff (now);
	return getDelayUntilAccessAllowed (now) + m_backoffLeft;
}

double 
Backoff::getDelayUntilAccessAllowed (double now) const
{
	double retval = getAccessAllowedStart () - now;
	retval = max (retval, 0.0);
	return retval;
}
void
Backoff::cancel (void)
{
	TRACE ("cancel");
	m_backoffLeft = 0.0;
}
void
Backoff::start (double backoffStart, double backoffDuration)
{
	TRACE ("start at %f for %f", backoffStart, backoffDuration);
	assert (m_backoffStart <= backoffStart);
	m_backoffStart = backoffStart;
	m_backoffLeft = backoffDuration;
}
bool
Backoff::isNavZero (double now) const
{
	double lastNavEnd = m_lastNavStart + m_lastNavDuration;
	if (m_lastNavStart <= now && 
	    lastNavEnd >= now &&
	    m_lastNavDuration > 0.0) {
		return false;
	} else {
		return true;
	}
}
double 
Backoff::getDelayUntilNavZero (double now) const
{
	if (isNavZero (now)) {
		return 0.0;
	}

	return m_lastNavStart + m_lastNavDuration - now;
}
void
Backoff::notifyNav (double navStart, double duration)
{
	assert (m_lastNAVStart < now);
	updateBackoff (navStart);
	double lastNavEnd = m_lastNavStart + m_lastNavDuration;
	double navEnd = navStart + duration;
	if (lastNavEnd > navStart) {
		/* The two NAVs overlap */
		if (navEnd > lastNavEnd) {
			m_lastNavDuration += navEnd - lastNavEnd;
		}
	} else {
		m_lastNavStart = navStart;
		m_lastNavDuration = duration;
	}
	TRACE ("nav start at %f for %f", m_lastNavStart, m_lastNavDuration);
}
void 
Backoff::notifyRxStart (double rxStart, double duration)
{
	TRACE ("rx start at %f for %f", rxStart, duration);
	updateBackoff (rxStart);
	m_lastRxStart = rxStart;
	m_lastRxDuration = duration;
	m_rxing = true;
}
void 
Backoff::notifyRxEnd (double rxEnd, bool receivedOk)
{
	TRACE ("rx end at %f -- %s", rxEnd, receivedOk?"ok":"failed");
	m_lastRxEnd = rxEnd;
	m_lastRxReceivedOk = receivedOk;
	m_rxing = false;
}
void 
Backoff::notifyTxStart (double txStart, double duration)
{
	TRACE ("tx start at %f for %f", txStart, duration);
	updateBackoff (txStart);
	m_lastTxStart = txStart;
	m_lastTxDuration = duration;
}
void 
Backoff::notifySleep (double now)
{
	TRACE ("sleep at %f", now);
	m_lastSleepStart = now;
	m_sleeping = true;
}
void 
Backoff::notifyWakeup (double now)
{
	TRACE ("wakeup at %f", now);
	m_lastWakeupStart = now;
	m_sleeping = false;
}
double
Backoff::getAccessAllowedStart (void) const
{
	/* This method evaluates the time where access to the
	 * medium is allowed. The return value could be 
	 * somewhere in the past or in the future.
	 */
	double rxAccessStart;
	if (m_lastRxEnd >= m_lastRxStart) {
		if (m_lastRxReceivedOk) {
			rxAccessStart = m_lastRxEnd + getDIFS ();
		} else {
			rxAccessStart = m_lastRxEnd + getEIFS ();
		}
	} else {
		rxAccessStart = m_lastRxStart + m_lastRxDuration + getDIFS ();
	}
	double txAccessStart = m_lastTxStart + m_lastTxDuration + getDIFS ();
	double navAccessStart = m_lastNavStart + m_lastNavDuration + getDIFS ();
	double accessAllowedStart = mostRecent (rxAccessStart,
						txAccessStart,
						navAccessStart);
	return accessAllowedStart;
}
void
Backoff::updateBackoff (double time)
{
	if (m_sleeping) {
		return;
	} else if (m_backoffLeft <= 0) {
		return;
	}
	
	assert (time >= m_backoffStart);

	double mostRecentEvent = max (m_backoffStart,
				      getAccessAllowedStart ());
	if (mostRecentEvent < time) {
		m_backoffLeft -= time - mostRecentEvent;
		m_backoffLeft = max (m_backoffLeft, 0.0); 
		m_backoffLeft = PRECISON_ROUND_TO_ZERO (m_backoffLeft);
		TRACE ("at %f left %f", time, m_backoffLeft);
		m_backoffStart = time;
	}
}

double
Backoff::getDIFS (void) const
{
	// XXX
	return m_mac->getDIFS ();
}
double
Backoff::getEIFS (void) const
{
	// XXX
	return m_mac->getEIFS ();
}

double
Backoff::max (double a, double b) const
{
	if (a > b) {
		return a;
	} else {
		return b;
	}
}

double
Backoff::mostRecent (double a, double b, double c) const
{
	double retval;
	retval = max (a, b);
	retval = max (retval, c);
	return retval;
}

