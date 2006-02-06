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
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include <iostream>
#include <cassert>
#include <math.h>

#include "dcf.h"
#include "phy-80211.h"
#include "mac-low.h"
#include "precision.h"
#include "mac-dcf-parameters.h"
#include "rng-uniform.h"
#include "mac-parameters.h"
#include "mac-traces.h"
#include "net-interface-80211.h"


#ifndef DCF_TRACE
#define nopeDCF_TRACE 1
#endif /* DCF_TRACE */

#ifdef DCF_TRACE
# define TRACE(format, ...) \
  printf ("DCF %d %f " format "\n", m_interface->getMacAddress (), \
          Scheduler::instance ().clock (), ## __VA_ARGS__);
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
	virtual void navReset (double now) {
		m_dcf->navReset (now);
	}
	virtual void navStart (double now, double duration) {
		m_dcf->navStart (now, duration);
	}
	virtual void navContinue (double duration) {
		m_dcf->navContinue (duration);
	}
private:
	Dcf *m_dcf;
};



Dcf::Dcf (MacDcfParameters *parameters)
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
	m_accessTimer = new DynamicHandler<Dcf> (this, &Dcf::accessTimeout);
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

MacDcfParameters *
Dcf::parameters (void)
{
	return m_parameters;
}

void
Dcf::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
	m_interface->low ()->registerNavListener (m_navListener);
	m_interface->phy ()->registerListener (m_phyListener);
}

void 
Dcf::requestAccess (void)
{
	double delayUntilAccessGranted  = getDelayUntilAccessGranted (now ());
	if (m_listener->accessingAndWillNotify ()) {
		/* don't do anything. We will start a backoff and maybe
		 * a timer when the txop notifies us of the end-of-access.
		 */
		TRACE ("accessing. will be notified.");
	} else if (m_accessTimer->isRunning ()) {
		/* we don't need to do anything because we have an access
		 * timer which will expire soon.
		 */
		TRACE ("access timer running. will be notified");
	} else if (isBackoffNotCompleted (now ()) && !m_accessTimer->isRunning ()) {
		/* start timer for ongoing backoff.
		 */
		TRACE ("request access X delayed for %f", delayUntilAccessGranted);
		m_accessTimer->start (delayUntilAccessGranted);
	} else if (m_interface->phy ()->getState () != Phy80211::IDLE) {
		/* someone else has accessed the medium.
		 * generate a backoff, start timer.
		 */
		startBackoff ();
	} else if (delayUntilAccessGranted > 0) {
		/* medium is IDLE, we have no backoff running but we 
		 * need to wait a bit before accessing the medium.
		 */
		TRACE ("request access Y delayed for %f", delayUntilAccessGranted);
		assert (!m_accessTimer->isRunning ());
		m_accessTimer->start (delayUntilAccessGranted);
	} else {
		/* we can access the medium now.
		 */
		TRACE ("access granted immediatly");
		m_listener->accessGrantedNow ();
	}
}

void
Dcf::notifyAccessFinished (void)
{
	TRACE ("access finished");
	startBackoff ();
}

void 
Dcf::notifyAccessOngoingOk (void)
{
	TRACE ("access ok");
	resetCW ();
}

void
Dcf::notifyAccessOngoingError (void)
{
	TRACE ("access failed");
	updateFailedCW ();
}
void
Dcf::notifyAccessOngoingErrorButOk (void)
{
	TRACE ("access failed but ok");
	resetCW ();
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
Dcf::accessTimeout (MacCancelableEvent *event)
{
	double delayUntilAccessGranted  = getDelayUntilAccessGranted (now ());
	if (delayUntilAccessGranted > 0) {
		TRACE ("timeout access delayed for %f", delayUntilAccessGranted);
		assert (!m_accessTimer->isRunning ());
		m_accessTimer->start (delayUntilAccessGranted);
	} else {
		TRACE ("timeout access granted");
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
	TRACE ("oooh %f, CW: %d<%d<%d, picked: %f", 
	       oooh, 
	       m_parameters->getCWmin (),
	       m_CW, 
	       m_parameters->getCWmax (), 
	       pickedCW);
	double delay =  pickedCW * 
		m_interface->parameters ()->getSlotTime ();
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
	return m_parameters->getEIFS (m_interface->phy ());
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
	assert (m_backoffLeft >= 0);
	retval += m_backoffLeft;
	retval = PRECISION_ROUND_TO_ZERO (retval);
	assert (retval >= 0);
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
Dcf::navReset (double navReset)
{
	double previousDelayUntilAccessGranted = getDelayUntilAccessGranted (navReset);
	m_lastNavStart = navReset;
	m_lastNavDuration = 0.0;
	double newDelayUntilAccessGranted = getDelayUntilAccessGranted (navReset);
	if (newDelayUntilAccessGranted < previousDelayUntilAccessGranted &&
	    m_accessTimer->isRunning () &&
	    m_accessTimer->getEndTime () > newDelayUntilAccessGranted) {
		/* This is quite unfortunate but we need to cancel the access timer
		 * because it seems that this NAV reset has brought the time of
		 * possible access closer to us than expected.
		 */
		m_accessTimer->cancel ();
		m_accessTimer->start (newDelayUntilAccessGranted);
	}
}
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
	TRACE ("sleep");
	m_lastSleepStart = now;
	m_sleeping = true;
}
void 
Dcf::notifyWakeup (double now)
{
	TRACE ("wakeup");
	m_lastWakeupStart = now;
	m_sleeping = false;
}
