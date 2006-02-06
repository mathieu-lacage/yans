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
#include <cassert>

#include "dcf.h"
#include "phy-80211.h"
#include "mac-low.h"
#include "precision.h"
#include "mac-dcf-parameters.h"
#include "rng-uniform.h"
#include "mac-container.h"
#include "mac-parameters.h"


#define nopeDCF_TRACE 1

#ifdef DCF_TRACE
# define TRACE(format, ...) \
  printf ("DCF %d " format "\n", m_container->selfAddress (), ## __VA_ARGS__);
#else /* DCF_TRACE */
# define TRACE(format, ...)
#endif /* DCF_TRACE */


static RngUniform *g_random = new RngUniform ();


DcfAccessListener::DcfAccessListener ()
{}
DcfAccessListener::~DcfAccessListener ()
{}


class DcfPhyListener : public Phy80211Listener
{
public:
	DcfPhyListener (Dcf *dcf) 
		: m_dcf (dcf) {}
	virtual void notifyRxStart (double now, double duration) {
		m_dcf->notifyRxStart (now, duration);
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

class DcfNavListener : public MacLowNavListener 
{
public:
	DcfNavListener (Dcf *dcf)
		: m_dcf (dcf) {}
	virtual void navStart (double now, double duration) {
		m_dcf->navStart (now, duration);
	}
	virtual void navContinue (double duration) {
		m_dcf->navContinue (duration);
	}
private:
	Dcf *m_dcf;
};



Dcf::Dcf (MacContainer *container, MacDcfParameters *parameters)
	: m_container (container),
	  m_parameters (parameters),
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
	Phy80211 *phy = container->phy ();
	MacLow *low = container->macLow ();
	m_phyListener = new DcfPhyListener (this);
	m_navListener = new DcfNavListener (this);
	m_accessTimer = new StaticHandler<Dcf> (this, &Dcf::accessTimeout);
	low->registerNavListener (m_navListener);
	phy->registerListener (m_phyListener);
	resetCW ();
	//m_random = new RngUniform ();
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
	double delayUntilAccessGranted  = getDelayUntilAccessGranted (now ());
	if (m_listener->accessingAndWillNotify ()) {
		/* don't do anything. We will start a backoff and maybe
		 * a timer when the txop notifies us of the end-of-access.
		 */
		TRACE ("accessing. will be notified.");
	} else if (isBackoffNotCompleted (now ()) && !m_accessTimer->isRunning ()) {
		/* start timer for ongoing backoff.
		 */
		m_accessTimer->start (delayUntilAccessGranted);
		TRACE ("access delayed for %f at %f", delayUntilAccessGranted, now ());
	} else if (m_container->phy ()->getState () != Phy80211::IDLE) {
		/* someone else has accessed the medium.
		 * generate a backoff, start timer.
		 */
		startBackoff ();
	} else if (delayUntilAccessGranted > 0) {
		/* medium is IDLE, we have no backoff running but we 
		 * need to wait a bit before accessing the medium.
		 */
		m_accessTimer->start (delayUntilAccessGranted);
		TRACE ("access delayed for %f at %f", delayUntilAccessGranted, now ());
	} else {
		/* we can access the medium now.
		 */
		TRACE ("access granted immediatly");
		m_listener->accessGrantedNow ();
	}
}

void 
Dcf::notifyAccessOk (void)
{
	TRACE ("access ok");
	resetCW ();
	startBackoff ();
}

void
Dcf::notifyAccessFailed (void)
{
	TRACE ("access failed");
	updateFailedCW ();
	startBackoff ();
}

void 
Dcf::registerAccessListener (DcfAccessListener *listener)
{
	m_listener = listener;
}

/***************************************************************
 *     Timeout method. Notifies when Access is Granted.
 ***************************************************************/ 


void 
Dcf::accessTimeout (void)
{
	double delayUntilAccessGranted  = getDelayUntilAccessGranted (now ());
	if (delayUntilAccessGranted > 0) {
		TRACE ("timeout access delayed for %f at %f", delayUntilAccessGranted, now ());
		m_accessTimer->start (delayUntilAccessGranted);
	} else {
		TRACE ("timeout access granted at %f", now ());
		m_listener->accessGrantedNow ();
	}
}


/***************************************************************
 *     Random trivial helper methods.
 ***************************************************************/ 

double
Dcf::now (void) const
{
	double now;
	now = Scheduler::instance ().clock ();
	return now;
}

double
Dcf::pickBackoffDelay (void)
{
	double oooh = g_random->pick ();
	double pickedCW = floor (oooh * m_CW);
	TRACE ("oooh %f, CW: %d, picked: %f", oooh, m_CW, pickedCW);
	double delay =  pickedCW * 
		m_container->parameters ()->getSlotTime ();
	return delay;
}
void
Dcf::resetCW (void)
{
	m_CW = m_parameters->getCWmin ();
}
void
Dcf::updateFailedCW (void)
{
	int CW = m_CW;
	CW *= 2;
	if (CW > m_parameters->getCWmax ()) {
		CW = m_parameters->getCWmax ();
	}

	m_CW = CW;
}

double 
Dcf::mostRecent (double a, double b) const
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
	return m_parameters->getAIFS ();
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
Dcf::startBackoff (void)
{
	double backoffStart = now ();
	double backoffDuration = pickBackoffDelay ();
	assert (m_backoffStart <= backoffStart);
	m_backoffStart = backoffStart;
	m_backoffLeft = backoffDuration;
	if (m_listener->accessNeeded () && !m_accessTimer->isRunning ()) {
		double delayUntilAccessGranted  = getDelayUntilAccessGranted (now ());
		if (delayUntilAccessGranted > 0) {
			TRACE ("start at %f for %f", backoffStart, backoffDuration);
			m_accessTimer->start (delayUntilAccessGranted);
		} else {
			TRACE ("access granted now");
			m_listener->accessGrantedNow ();
		}
	} else {
		if (m_accessTimer->isRunning ()) {
			TRACE ("no access needed because timer running.");
		} 
		if (m_listener->accessNeeded ()) {
			TRACE ("no access needed.");
		} 
		TRACE ("BUG");
	}
}
double
Dcf::getAccessGrantedStart (void) const
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
	double accessGrantedStart = mostRecent (rxAccessStart,
						txAccessStart,
						navAccessStart);
	return accessGrantedStart;
}

bool
Dcf::isBackoffNotCompleted (double now)
{
	updateBackoff (now);
	if (m_backoffLeft > 0) {
		return true;
	} else {
		return false;
	}
}


double 
Dcf::getDelayUntilAccessGranted (double now)
{
	double retval = getAccessGrantedStart () - now;
	retval = max (retval, 0.0);
	updateBackoff (now);
	retval += m_backoffLeft;
	retval = PRECISION_ROUND_TO_ZERO (retval);
	if (retval < 0) {
		char *p = 0;
		*p = 1;
	}
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
	
	//TRACE ("time: %f, backoffstart: %f\n", time, m_backoffStart);
	assert (time >= m_backoffStart);

	double mostRecentEvent = max (m_backoffStart,
				      getAccessGrantedStart ());
	if (mostRecentEvent < time) {
		m_backoffLeft -= time - mostRecentEvent;
		m_backoffLeft = max (m_backoffLeft, 0.0); 
		m_backoffLeft = PRECISION_ROUND_TO_ZERO (m_backoffLeft);
		TRACE ("at %f left %f", time, m_backoffLeft);
		m_backoffStart = time;
	}
}

/***************************************************************
 *     Notification methods.
 ***************************************************************/ 
void
Dcf::navStart (double navStart, double duration)
{
	assert (m_lastNavStart < navStart);
	TRACE ("nav start at %f for %f", navStart, duration);
	updateBackoff (navStart);
	m_lastNavStart = navStart;
	m_lastNavDuration = duration;
}
void
Dcf::navContinue (double duration)
{
	m_lastNavDuration += duration;
	TRACE ("nav continue for %f", duration);
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
