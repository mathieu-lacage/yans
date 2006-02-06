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
#include "mac-low.h"
#include "precision.h"
#include "mac-low-parameters.h"


#define nopeDCF_TRACE 1

#ifdef DCF_TRACE
# define TRACE(format, ...) \
  printf ("DCF %d " format "\n", m_mac->getSelf (), ## __VA_ARGS__);
#else /* DCF_TRACE */
# define TRACE(format, ...)
#endif /* DCF_TRACE */

class DcfPhyListener : public Phy80211Listener
{
public:
	DcfPhyListener (Dcf *dcf) 
		: m_backoff (dcf) {}
	virtual void notifyRxStart (double now, double duration) {
		m_dcf->notifyTxStart (now, duration);
	}
	virtual void notifyRxEnd (double now, bool receivedOk) {
		m_dcf->notifyRxEnd (now, receivedOk);
	}
	virtual void notifyTxStart (double now, double duration) {
		m_dcf->notifyTxStart (now, duration);
	}
	virtual void notifySleep (double now) {
		m_dcf->notifySleep (now);
	}
	virtual void notifyWakeup (double now) {
		m_dcf->notifyWakeup (now);
	}
private:
	Dcf *m_dcf;
};

class DcfNavListener : public NavListener 
{
public:
	DcfNavListener (Dcf *dcf)
		: m_dcf (dcf) {}
	virtual void notifyNav (double now, double duration) {
		m_dcf->notifyNav (now, duration);
	}
private:
	Dcf *m_dcf;
};



Dcf::Dcf (Phy80211 *phy, MacLow *mac, MacLowParameters *parameters)
	: m_parameters (parameters),
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
{
	m_phyListener = new DcfPhyListener (this);
	m_navListener = new DcfNavListener (this);
	mac->registerNavListener (m_navListener);
	phy->registerListener (m_phyListener);
}

Dcf::~Dcf ()
{
	delete m_phyListener;
	delete m_navListener;
}

/***************************************************************
 *     public API.
 ***************************************************************/ 


void 
Dcf::requestAccess (void)
{
	double delayUntilAccessAllowed  = getDelayUntilAccessAllowed ();
	if (delayUntilAccessAllowed > 0) {
		if (!m_accessTimer->isRunning ()) {
			// we did not start yet an access timer for this
			// backoff so we do it now.
			m_accessTimer->start (getDelayUntilAccessAllowed ());
		}
	} else {
		m_listener->accessGrantedNow ();
	}
}

void 
Dcf::notifyAccessOk (void)
{
	resetCW ();
	recordDcfStarted (now (), pickDcfDelay ());
}

void
Dcf::notifyAccessFailed (void)
{
	updateFailedCW ();
	recordDcfStarted (now (), pickDcfDelay ());
}

void 
Dcf::registerAcessListener (DcfAccessListener *listener)
{
	m_listener = listener;
}

/***************************************************************
 *     Timeout method. Notifies when Access is Granted.
 ***************************************************************/ 

void 
Dcf::accessTimeout (void)
{
	m_listener->accessGrantedNow ();
}


/***************************************************************
 *     Random trivial helper methods.
 ***************************************************************/ 


double
Dcf::pickDcfDelay (void)
{
	double delay = floor (m_random->pick () * m_CW) * parameters ()->getSlotTime ();
	return delay;
}
void
Dcf::resetCW (void)
{
	m_CW = parameters ()->getCWmin ();
}
void
Dcf::updateFailedCW (void)
{
	double CW = m_CW;
	CW *= 2;
	if (CW > parameters ()->getCWmax ()) {
		CW = parameters ()->getCWmax ();
	}

	m_CW = CW;
}

double 
Dcf::mostRecent (double a, double b)
{
	if (a >= b) {
		return a;
	} else {
		return b;
	}
}
double
Dcf::mostRecent (double a, double b, double c) const
{
	double retval;
	retval = max (a, b);
	retval = max (retval, c);
	return retval;
}


double
Dcf::getDIFS (void) const
{
	return m_parameters->getDIFS ();
}
double
Dcf::getEIFS (void) const
{
	return m_parameters->getEIFS ();
}




/***************************************************************
 *     Complicated timekeeping backoff methods.
 ***************************************************************/ 

void
Dcf::recordDcfStarted (double backoffStart, double backoffDuration)
{
	TRACE ("start at %f for %f", backoffStart, backoffDuration);
	assert (m_backoffStart <= backoffStart);
	m_backoffStart = backoffStart;
	m_backoffLeft = backoffDuration;
}
double
Dcf::getAccessAllowedStart (void) const
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


double 
Dcf::getDelayUntilAccessAllowed (double now) const
{
	updateBackoff (now);
	double retval = getAccessAllowedStart () - now;
	retval = max (retval, 0.0);
	retval += m_backoffLeft;
	return retval;
}
void
Dcf::updateBackoff (double time)
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

/***************************************************************
 *     Notification methods.
 ***************************************************************/ 
void
Dcf::notifyNav (double navStart, double duration)
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
Dcf::notifyRxStart (double rxStart, double duration)
{
	TRACE ("rx start at %f for %f", rxStart, duration);
	updateBackoff (rxStart);
	m_lastRxStart = rxStart;
	m_lastRxDuration = duration;
	m_rxing = true;
}
void 
Dcf::notifyRxEnd (double rxEnd, bool receivedOk)
{
	TRACE ("rx end at %f -- %s", rxEnd, receivedOk?"ok":"failed");
	m_lastRxEnd = rxEnd;
	m_lastRxReceivedOk = receivedOk;
	m_rxing = false;
}
void 
Dcf::notifyTxStart (double txStart, double duration)
{
	TRACE ("tx start at %f for %f", txStart, duration);
	updateBackoff (txStart);
	m_lastTxStart = txStart;
	m_lastTxDuration = duration;
}
void 
Dcf::notifySleep (double now)
{
	TRACE ("sleep at %f", now);
	m_lastSleepStart = now;
	m_sleeping = true;
}
void 
Dcf::notifyWakeup (double now)
{
	TRACE ("wakeup at %f", now);
	m_lastWakeupStart = now;
	m_sleeping = false;
}
