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


#define nopeDCF_TRACE 1

#ifdef DCF_TRACE
#  include <iostream>
#  define TRACE(x) \
   std::cout << "DCF " << x << std::endl;
#else /* DCF_TRACE */
# define TRACE(x)
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
	virtual void notify_rx_start (double now, double duration) {
		m_dcf->notify_rx_start (now, duration);
	}
	virtual void notify_rx_end (double now, bool receivedOk) {
		m_dcf->notify_rx_end (now, receivedOk);
	}
	virtual void notify_tx_start (double now, double duration) {
		m_dcf->notify_tx_start (now, duration);
	}
	virtual void notify_sleep (double now) {
		m_dcf->notify_sleep (now);
	}
	virtual void notify_wakeup (double now) {
		m_dcf->notify_wakeup (now);
	}
private:
	Dcf *m_dcf;
};

class DcfNavListener : public MacLowNavListener 
{
public:
	DcfNavListener (Dcf *dcf)
		: m_dcf (dcf) {}
	virtual void nav_reset (double now, double duration) {
		m_dcf->nav_reset (now, duration);
	}
	virtual void nav_start (double now, double duration) {
		m_dcf->nav_start (now, duration);
	}
	virtual void nav_continue (double duration) {
		m_dcf->nav_continue (duration);
	}
private:
	Dcf *m_dcf;
};



Dcf::_dcf (MacDcfParameters *parameters)
	: m_parameters (parameters),
	  m_backoff_start (0.0),
	  m_backoff_left (0.0),
	  m_last_nav_start (0.0),
	  m_last_nav_duration (0.0),
	  m_last_rx_start (0.0),
	  m_last_rx_duration (0.0),
	  m_last_rx_received_ok (true),
	  m_last_rx_end (0.0),
	  m_last_tx_start (0.0),
	  m_last_tx_duration (0.0),
	  m_last_sleep_start (0.0),
	  m_last_wakeup_start (0.0),
	  m_rxing (false),
	  m_sleeping (false)
{
	m_phy_listener = new DcfPhyListener (this);
	m_nav_listener = new DcfNavListener (this);
	m_access_timer = new DynamicHandler<Dcf> (this, &Dcf::accessTimeout);
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
Dcf::set_interface (NetInterface80211 *interface)
{
	m_interface = interface;
	m_interface->low ()->registerNavListener (m_navListener);
	m_interface->phy ()->register_listener (m_phyListener);
}

void 
Dcf::request_access (void)
{
	double delayUntilAccessGranted  = get_delay_until_access_granted (now ());
	if (m_listener->accessing_and_will_notify ()) {
		/* don't do anything. We will start a backoff and maybe
		 * a timer when the txop notifies us of the end-of-access.
		 */
		TRACE ("accessing. will be notified.");
	} else if (m_access_timer->is_running ()) {
		/* we don't need to do anything because we have an access
		 * timer which will expire soon.
		 */
		TRACE ("access timer running. will be notified");
	} else if (isBackoffNotCompleted (now ()) && !m_access_timer->isRunning ()) {
		/* start timer for ongoing backoff.
		 */
		TRACE ("request access X delayed for %f", delayUntilAccessGranted);
		m_access_timer->start (delayUntilAccessGranted);
	} else if (m_interface->phy ()->get_state () != Phy80211::IDLE) {
		/* someone else has accessed the medium.
		 * generate a backoff, start timer.
		 */
		startBackoff ();
	} else if (delayUntilAccessGranted > 0) {
		/* medium is IDLE, we have no backoff running but we 
		 * need to wait a bit before accessing the medium.
		 */
		TRACE ("request access Y delayed for %f", delayUntilAccessGranted);
		assert (!m_access_timer->is_running ());
		m_access_timer->start (delayUntilAccessGranted);
	} else {
		/* we can access the medium now.
		 */
		TRACE ("access granted immediatly");
		m_listener->accessGrantedNow ();
	}
}

void
Dcf::notify_access_finished (void)
{
	TRACE ("access finished");
	startBackoff ();
}

void 
Dcf::notify_access_ongoing_ok (void)
{
	TRACE ("access ok");
	resetCW ();
}

void
Dcf::notify_access_ongoing_error (void)
{
	TRACE ("access failed");
	updateFailedCW ();
}
void
Dcf::notify_access_ongoing_error_but_ok (void)
{
	TRACE ("access failed but ok");
	resetCW ();
}

void 
Dcf::register_access_listener (DcfAccessListener *listener)
{
	m_listener = listener;
}

/***************************************************************
 *     Timeout method. Notifies when Access is Granted.
 ***************************************************************/ 


void 
Dcf::access_timeout (MacCancelableEvent *event)
{
	double delayUntilAccessGranted  = get_delay_until_access_granted (now ());
	if (delayUntilAccessGranted > 0) {
		TRACE ("timeout access delayed for %f", delayUntilAccessGranted);
		assert (!m_access_timer->is_running ());
		m_access_timer->start (delayUntilAccessGranted);
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
Dcf::pick_backoff_delay (void)
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
Dcf::reset_cw (void)
{
	m_CW = m_parameters->getCWmin ();
}
void
Dcf::update_failed_cw (void)
{
	int CW = m_CW;
	CW *= 2;
	if (CW > m_parameters->get_cwmax ()) {
		CW = m_parameters->getCWmax ();
	}

	m__cw = CW;
}

double 
Dcf::most_recent (double a, double b) const
{
	if (a >= b) {
		return a;
	} else {
		return b;
	}
}
double
Dcf::most_recent (double a, double b, double c) const
{
	double retval;
	retval = max (a, b);
	retval = max (retval, c);
	return retval;
}


double
Dcf::get_difs (void) const
{
	return m_parameters->getAIFS ();
}
double
Dcf::get_eifs (void) const
{
	return m_parameters->getEIFS (m_interface->phy ());
}

/***************************************************************
 *     Complicated timekeeping backoff methods.
 ***************************************************************/ 

void
Dcf::start_backoff (void)
{
	double backoffStart = now ();
	double backoffDuration = pickBackoffDelay ();
	assert (m_backoff_start <= backoffStart);
	m_backoff_start = backoffStart;
	m_backoff_left = backoffDuration;
	if (m_listener->accessNeeded () && !m_access_timer->is_running ()) {
		double delayUntilAccessGranted  = get_delay_until_access_granted (now ());
		if (delayUntilAccessGranted > 0) {
			TRACE ("start at %f for %f", backoffStart, backoffDuration);
			m_access_timer->start (delayUntilAccessGranted);
		} else {
			TRACE ("access granted now");
			m_listener->accessGrantedNow ();
		}
	} else {
		if (m_access_timer->is_running ()) {
			TRACE ("no access needed because timer running.");
		} 
		if (!m_listener->access_needed ()) {
			TRACE ("no access needed.");
		}
		TRACE ("no access needed for now.");
	}
}
double
Dcf::get_access_granted_start (void) const
{
	/* This method evaluates the time where access to the
	 * medium is allowed. The return value could be 
	 * somewhere in the past or in the future.
	 */
	double rxAccessStart;
	if (m_lastRxEnd >= m_lastRxStart) {
		if (m_lastRxReceivedOk) {
			rxAccessStart = m_last_rx_end + getDIFS ();
		} else {
			rxAccessStart = m_last_rx_end + getEIFS ();
		}
	} else {
		rxAccessStart = m_lastRxStart + m_last_rx_duration + getDIFS ();
	}
	double txAccessStart = m_lastTxStart + m_last_tx_duration + getDIFS ();
	double navAccessStart = m_lastNavStart + m_last_nav_duration + getDIFS ();
	double accessGrantedStart = mostRecent (rxAccessStart,
						txAccessStart,
						navAccessStart);
	return accessGrantedStart;
}

bool
Dcf::is_backoff_not_completed (double now)
{
	updateBackoff (now);
	if (m_backoffLeft > 0) {
		return true;
	} else {
		return false;
	}
}


double 
Dcf::get_delay_until_access_granted (double now)
{
	double retval = getAccessGrantedStart () - now;
	retval = max (retval, 0.0);
	updateBackoff (now);
	assert (m_backoff_left >= 0);
	retval += m_backoffLeft;
	retval = PRECISION_ROUND_TO_ZERO (retval);
	assert (retval >= 0);
	return retval;
}
void
Dcf::update_backoff (double time)
{
	if (m_sleeping) {
		return;
	} else if (m_backoffLeft <= 0) {
		return;
	}
	
	//TRACE ("time: %f, backoffstart: %f\n", time, m_backoffStart);
	assert (time >= m_backoffStart);

	double mostRecentEvent = max (m_backoffStart,
				      get_access_granted_start ());
	if (mostRecentEvent < time) {
		m_backoff_left -= time - mostRecentEvent;
		m_backoff_left = max (m_backoffLeft, 0.0); 
		m_backoff_left = PRECISION_ROUND_TO_ZERO (m_backoffLeft);
		TRACE ("at %f left %f", time, m_backoffLeft);
		m_backoff_start = time;
	}
}

/***************************************************************
 *     Notification methods.
 ***************************************************************/ 
void
Dcf::nav_reset (double navStart, double duration)
{
	double navEnd = navStart + duration;
	double previousDelayUntilAccessGranted = get_delay_until_access_granted (navEnd);
	m_last_nav_start = navEnd;
	m_last_nav_duration = 0.0;
	double newDelayUntilAccessGranted = get_delay_until_access_granted (navEnd);
	if (newDelayUntilAccessGranted < previousDelayUntilAccessGranted &&
	    m_access_timer->isRunning () &&
	    m_access_timer->get_end_time () > newDelayUntilAccessGranted) {
		/* This is quite unfortunate but we need to cancel the access timer
		 * because it seems that this NAV reset has brought the time of
		 * possible access closer to us than expected.
		 */
		m_access_timer->cancel ();
		m_access_timer->start (newDelayUntilAccessGranted);
	}
}
void
Dcf::nav_start (double navStart, double duration)
{
	assert (m_last_nav_start < navStart);
	TRACE ("nav start at %f for %f", navStart, duration);
	updateBackoff (navStart);
	m_last_nav_start = navStart;
	m_last_nav_duration = duration;
}
void
Dcf::nav_continue (double duration)
{
	m_last_nav_duration += duration;
	TRACE ("nav continue for %f", duration);
}

void 
Dcf::notify_rx_start (double rxStart, double duration)
{
	TRACE ("rx start at %f for %f", rxStart, duration);
	updateBackoff (rxStart);
	m_last_rx_start = rxStart;
	m_last_rx_duration = duration;
	m_rxing = true;
}
void 
Dcf::notify_rx_end (double rxEnd, bool receivedOk)
{
	TRACE ("rx end at %f -- %s", rxEnd, receivedOk?"ok":"failed");
	m_last_rx_end = rxEnd;
	m_last_rx_received_ok = receivedOk;
	m_rxing = false;
}
void 
Dcf::notify_tx_start (double txStart, double duration)
{
	TRACE ("tx start at %f for %f", txStart, duration);
	updateBackoff (txStart);
	m_last_tx_start = txStart;
	m_last_tx_duration = duration;
}
void 
Dcf::notify_sleep (double now)
{
	TRACE ("sleep");
	m_last_sleep_start = now;
	m_sleeping = true;
}
void 
Dcf::notify_wakeup (double now)
{
	TRACE ("wakeup");
	m_last_wakeup_start = now;
	m_sleeping = false;
}
