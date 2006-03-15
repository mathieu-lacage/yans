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
#include "random-uniform.h"
#include "cancellable-event.tcc"
#include "simulator.h"
#include "mac-parameters.h"

#ifndef max
#define max(a,b) (((a)>=(b))?(a):(b))
#endif


#define nopeDCF_TRACE 1

#ifdef DCF_TRACE
#  include <iostream>
#  define TRACE(x) \
   std::cout << "DCF " << x << std::endl;
#else /* DCF_TRACE */
# define TRACE(x)
#endif /* DCF_TRACE */

namespace yans {


DcfAccessListener::DcfAccessListener ()
{}
DcfAccessListener::~DcfAccessListener ()
{}



Dcf::Dcf ()
	: m_backoff_start (0),
	  m_backoff_left (0),
	  m_last_nav_start (0),
	  m_last_nav_duration (0),
	  m_last_rx_start (0),
	  m_last_rx_duration (0),
	  m_last_rx_received_ok (true),
	  m_last_rx_end (0),
	  m_last_tx_start (0),
	  m_last_tx_duration (0),
	  m_last_sleep_start (0),
	  m_last_wakeup_start (0),
	  m_rxing (false),
	  m_sleeping (false)
{
	m_access_timer_event = 0;
	reset_cw ();
	m_random = new RandomUniform ();
}

Dcf::~Dcf ()
{
	delete m_random;
	delete m_listener;
}

void
Dcf::set_parameters (MacParameters const*parameters)
{
	m_parameters = parameters;
}

void 
Dcf::set_difs_us (uint64_t difs_us)
{
	m_difs_us = difs_us;
}
void 
Dcf::set_eifs_us (uint64_t eifs_us)
{
	m_eifs_us = eifs_us;
}
void 
Dcf::set_cw_bounds (uint32_t min, uint32_t max)
{
	m_cw_min = min;
	m_cw_max = max;
}

void 
Dcf::register_access_listener (DcfAccessListener *listener)
{
	m_listener = listener;
}


/***************************************************************
 *     public API.
 ***************************************************************/ 

void 
Dcf::request_access (bool is_phy_busy)
{
	uint64_t delay_until_access_granted = get_delay_until_access_granted (now_us ());
	if (m_listener->accessing_and_will_notify ()) {
		/* don't do anything. We will start a backoff and maybe
		 * a timer when the txop notifies us of the end-of-access.
		 */
		TRACE ("accessing. will be notified.");
	} else if (m_access_timer_event != 0) {
		/* we don't need to do anything because we have an access
		 * timer which will expire soon.
		 */
		TRACE ("access timer running. will be notified");
	} else if (is_backoff_not_completed (now_us ()) && m_access_timer_event == 0) {
		/* start timer for ongoing backoff.
		 */
		TRACE ("request access X delayed for "<<delay_until_access_granted);
		m_access_timer_event = make_cancellable_event (&Dcf::access_timeout, this);
		Simulator::insert_in_us (delay_until_access_granted, m_access_timer_event);
	} else if (is_phy_busy) {
		/* someone else has accessed the medium.
		 * generate a backoff, start timer.
		 */
		start_backoff ();
	} else if (delay_until_access_granted > 0) {
		/* medium is IDLE, we have no backoff running but we 
		 * need to wait a bit before accessing the medium.
		 */
		TRACE ("request access Y delayed for "<< delay_until_access_granted);
		assert (m_access_timer_event == 0);
		m_access_timer_event = make_cancellable_event (&Dcf::access_timeout, this);
		Simulator::insert_in_us (delay_until_access_granted, m_access_timer_event);
	} else {
		/* we can access the medium now.
		 */
		TRACE ("access granted immediatly");
		m_listener->access_granted_now ();
	}
}

void
Dcf::notify_access_finished (void)
{
	TRACE ("access finished");
	start_backoff ();
}

void 
Dcf::notify_access_ongoing_ok (void)
{
	TRACE ("access ok");
	reset_cw ();
}

void
Dcf::notify_access_ongoing_error (void)
{
	TRACE ("access failed");
	update_failed_cw ();
}
void
Dcf::notify_access_ongoing_error_but_ok (void)
{
	TRACE ("access failed but ok");
	reset_cw ();
}

/***************************************************************
 *     Timeout method. Notifies when Access is Granted.
 ***************************************************************/ 


void 
Dcf::access_timeout ()
{
	uint64_t delay_until_access_granted  = get_delay_until_access_granted (now_us ());
	if (delay_until_access_granted > 0) {
		TRACE ("timeout access delayed for "<< delay_until_access_granted);
		assert (m_access_timer_event == 0);
		m_access_timer_event = make_cancellable_event (&Dcf::access_timeout, this);
		Simulator::insert_in_us (delay_until_access_granted, m_access_timer_event);
	} else {
		TRACE ("timeout access granted");
		m_listener->access_granted_now ();
	}
}


/***************************************************************
 *     Random trivial helper methods.
 ***************************************************************/ 

uint64_t
Dcf::now_us (void) const
{
	uint64_t now = Simulator::now_us ();
	return now;
}

uint64_t
Dcf::pick_backoff_delay (void)
{
	uint32_t picked_cw = m_random->get_uint (0, m_cw);
	TRACE ("cw="<<get_cw_min ()<<
	       "<"<<m_cw<<"<"<<get_cw_max ()<<
	       ", picked="<<picked_cw); 
	uint64_t delay_us = picked_cw * m_parameters->get_slot_time_us ();
	return delay_us;
}
void
Dcf::reset_cw (void)
{
	m_cw = get_cw_min ();
}
void
Dcf::update_failed_cw (void)
{
	uint32_t cw = m_cw;
	cw *= 2;
	if (cw > get_cw_max ()) {
		cw = get_cw_max ();
	}
	m_cw = cw;
}

uint64_t
Dcf::most_recent (uint64_t a, uint64_t b) const
{
	return max (a, b);
}
uint64_t
Dcf::most_recent (uint64_t a, uint64_t b, uint64_t c) const
{
	uint64_t retval;
	retval = most_recent (a, b);
	retval = most_recent (retval, c);
	return retval;
}

uint64_t 
Dcf::get_difs_us (void) const
{
	return m_difs_us;
}
uint64_t 
Dcf::get_eifs_us (void) const
{
	return m_eifs_us;
}
uint32_t 
Dcf::get_cw_min (void) const
{
	return m_cw_min;
}
uint32_t 
Dcf::get_cw_max (void) const
{
	return m_cw_max;
}

/***************************************************************
 *     Complicated timekeeping backoff methods.
 ***************************************************************/ 

void
Dcf::start_backoff (void)
{
	uint64_t backoff_start = now_us ();
	uint64_t backoff_duration = pick_backoff_delay ();
	assert (m_backoff_start <= backoff_start);
	m_backoff_start = backoff_start;
	m_backoff_left = backoff_duration;
	if (m_listener->access_needed () && m_access_timer_event == 0) {
		uint64_t delay_until_access_granted  = get_delay_until_access_granted (now_us ());
		if (delay_until_access_granted > 0) {
			TRACE ("start at "<<backoff_start<<", for "<<backoff_duration);
			m_access_timer_event = make_cancellable_event (&Dcf::access_timeout, this);
			Simulator::insert_in_us (delay_until_access_granted, m_access_timer_event);
		} else {
			TRACE ("access granted now");
			m_listener->access_granted_now ();
		}
	} else {
		if (m_access_timer_event != 0) {
			TRACE ("no access needed because timer running.");
		} 
		if (!m_listener->access_needed ()) {
			TRACE ("no access needed.");
		}
		TRACE ("no access needed for now.");
	}
}
uint64_t
Dcf::get_access_granted_start (void) const
{
	/* This method evaluates the time where access to the
	 * medium is allowed. The return value could be 
	 * somewhere in the past or in the future.
	 */
	uint64_t rx_access_start;
	if (m_last_rx_end >= m_last_rx_start) {
		if (m_last_rx_received_ok) {
			rx_access_start = m_last_rx_end + get_difs_us ();
		} else {
			rx_access_start = m_last_rx_end + get_eifs_us ();
		}
	} else {
		rx_access_start = m_last_rx_start + m_last_rx_duration + get_difs_us ();
	}
	uint64_t tx_access_start = m_last_tx_start + m_last_tx_duration + get_difs_us ();
	uint64_t nav_access_start = m_last_nav_start + m_last_nav_duration + get_difs_us ();
	uint64_t access_granted_start = most_recent (rx_access_start, tx_access_start, nav_access_start);
	return access_granted_start;
}

bool
Dcf::is_backoff_not_completed (uint64_t now)
{
	update_backoff (now);
	if (m_backoff_left > 0) {
		return true;
	} else {
		return false;
	}
}


uint64_t
Dcf::get_delay_until_access_granted (uint64_t now)
{
	int64_t delta_to = get_access_granted_start () - now;
	uint64_t retval = max (delta_to, 0);
	update_backoff (now);
	retval += m_backoff_left;
	return retval;
}
void
Dcf::update_backoff (uint64_t time_us)
{
	if (m_sleeping || m_backoff_left == 0) {
		return;
	}
	
	//TRACE ("time: %f, backoffstart: %f\n", time, m_backoffStart);
	assert (time_us >= m_backoff_start);

	uint64_t most_recent_event = most_recent (m_backoff_start,
						  get_access_granted_start ());
	if (most_recent_event < time_us) {
		int64_t new_backoff_left = m_backoff_left - (time_us - most_recent_event);
		m_backoff_left = max (new_backoff_left, 0); 
		TRACE ("at="<<time_us<<", left="<< m_backoff_left);
		m_backoff_start = time_us;
	}
}

/***************************************************************
 *     Notification methods.
 ***************************************************************/ 
void
Dcf::nav_reset (uint64_t nav_start, uint64_t duration)
{
	m_last_nav_start = nav_start;
	m_last_nav_duration = duration;
	uint64_t nav_end = nav_start + duration;
	uint64_t new_delay_until_access_granted = get_delay_until_access_granted (nav_end);
	assert (new_delay_until_access_granted > 0);
	/* This is quite unfortunate but we need to cancel the access timer
	 * because this nav reset might have brought the time of
	 * possible access closer to us than expected.
	 */
	m_access_timer_event->cancel ();
	m_access_timer_event = 0;
	m_access_timer_event = make_cancellable_event (&Dcf::access_timeout, this);
	Simulator::insert_in_us (new_delay_until_access_granted, m_access_timer_event);
}
void
Dcf::nav_start (uint64_t nav_start, uint64_t duration)
{
	assert (m_last_nav_start < nav_start);
	TRACE ("nav start at="<<nav_start<<", for="<<duration);
	update_backoff (nav_start);
	m_last_nav_start = nav_start;
	m_last_nav_duration = duration;
}
void
Dcf::nav_continue (uint64_t duration)
{
	m_last_nav_duration += duration;
	TRACE ("nav continue for "<<duration);
}

void 
Dcf::notify_rx_start (uint64_t rx_start, uint64_t duration)
{
	TRACE ("rx start at="<<rx_start<<", for="<<duration);
	update_backoff (rx_start);
	m_last_rx_start = rx_start;
	m_last_rx_duration = duration;
	m_rxing = true;
}
void 
Dcf::notify_rx_end (uint64_t rx_end, bool received_ok)
{
	TRACE ("rx end at="<<rx_end<<" -- "<<received_ok?"ok":"failed");
	m_last_rx_end = rx_end;
	m_last_rx_received_ok = received_ok;
	m_rxing = false;
}
void 
Dcf::notify_tx_start (uint64_t tx_start, uint64_t duration)
{
	TRACE ("tx start at="<<tx_start<<" for "<<duration);
	update_backoff (tx_start);
	m_last_tx_start = tx_start;
	m_last_tx_duration = duration;
}
void 
Dcf::notify_sleep (uint64_t sleep_start)
{
	TRACE ("sleep");
	m_last_sleep_start = sleep_start;
	m_sleeping = true;
}
void 
Dcf::notify_wakeup (uint64_t sleep_end)
{
	TRACE ("wakeup");
	m_last_wakeup_start = sleep_end;
	m_sleeping = false;
}

}; // namespace yans
