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
#include "event.tcc"
#include "simulator.h"
#include "mac-parameters.h"
#include "trace-container.h"

#ifndef max
#define max(a,b) (((a)>=(b))?(a):(b))
#endif


#define noDCF_TRACE 1

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
	: m_access_timer_event (),
	  m_backoff_start (0),
	  m_backoff_left (0),
	  m_last_nav_start (0),
	  m_last_nav_duration (0),
	  m_last_rx_start (0),
	  m_last_rx_duration (0),
	  m_last_rx_received_ok (true),
	  m_last_rx_end (0),
	  m_last_tx_start (0),
	  m_last_tx_duration (0),
	  m_last_busy_start (0),
	  m_last_busy_duration (0),
	  m_rxing (false)
{
	reset_cw ();
	m_random = new RandomUniform ();
}

Dcf::~Dcf ()
{
	delete m_random;
}

void 
Dcf::reset_rng (uint32_t seed)
{
	m_random->reset (seed);
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
	m_cw = min;
}
void 
Dcf::register_traces (TraceContainer *container)
{
	container->register_ui_variable ("80211-dcf-cw", &m_cw);
	container->register_callback ("80211-dcf-backoff", &m_backoff_trace);
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
Dcf::request_access (void)
{
	uint64_t delay_until_access_granted = get_delay_until_access_granted (now_us ());
	if (m_listener->accessing_and_will_notify ()) {
		/* don't do anything. We will start a backoff and maybe
		 * a timer when the txop notifies us of the end-of-access.
		 */
		TRACE ("accessing. will be notified.");
	} else if (m_access_timer_event.is_running ()) {
		/* we don't need to do anything because we have an access
		 * timer which will expire soon.
		 */
		TRACE ("access timer running. will be notified");
	} else if (is_backoff_not_completed (now_us ()) && !m_access_timer_event.is_running ()) {
		/* start timer for ongoing backoff.
		 */
		TRACE ("request access X delayed for="<<delay_until_access_granted);
		m_access_timer_event = make_event (&Dcf::access_timeout, this);
		Simulator::schedule_rel_us (delay_until_access_granted, m_access_timer_event);
	} else if (is_phy_busy ()) {
		/* someone else has accessed the medium.
		 * generate a backoff, start timer.
		 */
		start_backoff ();
	} else if (delay_until_access_granted > 0) {
		/* medium is IDLE, we have no backoff running but we 
		 * need to wait a bit before accessing the medium.
		 */
		TRACE ("request access Y delayed for="<< delay_until_access_granted);
		assert (!m_access_timer_event.is_running ());
		m_access_timer_event = make_event (&Dcf::access_timeout, this);
		Simulator::schedule_rel_us (delay_until_access_granted, m_access_timer_event);
	} else {
		/* we can access the medium now.
		 */
		TRACE ("access granted immediatly");
		m_listener->access_granted_now ();
	}
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
		m_access_timer_event = make_event (&Dcf::access_timeout, this);
		Simulator::schedule_rel_us (delay_until_access_granted, m_access_timer_event);
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
Dcf::most_recent (uint64_t a, uint64_t b, uint64_t c, uint64_t d) const
{
	uint64_t e = max (a, b);
	uint64_t f = max (c, d);
	uint64_t retval = max (e, f);
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

bool 
Dcf::is_phy_busy (void)
{
	if (m_rxing) {
		return true;
	}
	uint64_t last_tx_end = m_last_tx_start + m_last_tx_duration;
	if (last_tx_end > Simulator::now_us ()) {
		return true;
	}
	return false;
}

void
Dcf::start_backoff (void)
{
	uint64_t backoff_start = now_us ();
	uint64_t backoff_duration = pick_backoff_delay ();
	m_backoff_trace (backoff_duration);
	assert (m_backoff_start <= backoff_start);
	m_backoff_start = backoff_start;
	m_backoff_left = backoff_duration;
	if (m_listener->access_needed () && !m_access_timer_event.is_running ()) {
		uint64_t delay_until_access_granted  = get_delay_until_access_granted (now_us ());
		if (delay_until_access_granted > 0) {
			TRACE ("start at "<<backoff_start<<", for "<<backoff_duration);
			m_access_timer_event = make_event (&Dcf::access_timeout, this);
			Simulator::schedule_rel_us (delay_until_access_granted, m_access_timer_event);
		} else {
			TRACE ("access granted now");
			m_listener->access_granted_now ();
		}
	} else {
		if (m_access_timer_event.is_running ()) {
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
	uint64_t busy_access_start = m_last_busy_start + m_last_busy_duration + get_difs_us ();
	uint64_t tx_access_start = m_last_tx_start + m_last_tx_duration + get_difs_us ();
	uint64_t nav_access_start = m_last_nav_start + m_last_nav_duration + get_difs_us ();
	uint64_t access_granted_start = most_recent (rx_access_start, 
						     busy_access_start,
						     tx_access_start, 
						     nav_access_start);
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
	if (m_backoff_left == 0) {
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
Dcf::notify_nav_reset (uint64_t nav_start, uint64_t duration)
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
	if (m_access_timer_event.is_running ()) {
		m_access_timer_event.cancel ();
		m_access_timer_event = make_event (&Dcf::access_timeout, this);
		Simulator::schedule_rel_us (new_delay_until_access_granted, m_access_timer_event);
	}
}
void
Dcf::notify_nav_start (uint64_t nav_start, uint64_t duration)
{
	assert (m_last_nav_start < nav_start);
	TRACE ("nav start at="<<nav_start<<", for="<<duration);
	update_backoff (nav_start);
	m_last_nav_start = nav_start;
	m_last_nav_duration = duration;
}
void
Dcf::notify_nav_continue (uint64_t nav_start, uint64_t duration)
{
	notify_nav_start (nav_start, duration);
}

void 
Dcf::notify_rx_start_now (uint64_t duration)
{
	uint64_t now = now_us ();
	TRACE ("rx start at="<<now<<", for="<<duration);
	update_backoff (now);
	m_last_rx_start = now;
	m_last_rx_duration = duration;
	m_rxing = true;
}
void 
Dcf::notify_rx_end_ok_now (void)
{
	uint64_t now = now_us ();
	TRACE ("rx end ok at="<<now);
	m_last_rx_end = now;
	m_last_rx_received_ok = true;
	m_rxing = false;
}
void 
Dcf::notify_rx_end_error_now (void)
{
	uint64_t now = now_us ();
	TRACE ("rx end error at="<<now);
	m_last_rx_end = now;
	m_last_rx_received_ok = false;
	m_rxing = false;
}
void 
Dcf::notify_tx_start_now (uint64_t duration)
{
	uint64_t now = now_us ();
	TRACE ("tx start at="<<now<<" for "<<duration);
	update_backoff (now);
	m_last_tx_start = now;
	m_last_tx_duration = duration;
}

void 
Dcf::notify_cca_busy_start_now (uint64_t duration)
{
	uint64_t now = now_us ();
	TRACE ("busy start at="<<now<<" for "<<duration);
	update_backoff (now);
	m_last_busy_start = now;
	m_last_busy_duration = duration;
}

}; // namespace yans

#ifdef RUN_SELF_TESTS
#include "test.h"
#include "event.tcc"
#include <list>

namespace yans {

class DcfTest : public Test {
public:
	DcfTest ();
	virtual bool run_tests (void);

	// callback from DcfListener
	void access_granted_now (void);
	bool access_needed (void);
	bool accessing_and_will_notify (void);
private:

	void add_rx_ok_evt (uint64_t at, uint64_t duration);
	void add_rx_error_evt (uint64_t at, uint64_t duration);
	void add_tx_evt (uint64_t at, uint64_t duration);
	void add_nav_reset (uint64_t at, uint64_t start, uint64_t duration);
	void add_nav_start (uint64_t at, uint64_t start, uint64_t duration);
	void add_nav_continue (uint64_t at, uint64_t start, uint64_t duration);
	void add_access_request (uint64_t time);
	void add_access_error (uint64_t time);
	void add_access_error_but_ok (uint64_t time);
	void add_access_ok (uint64_t time);

	void expect_access_granted (uint64_t time);
	
	// callback to forward to DCF
	void access_error (uint64_t time);
	void access_error_but_ok (uint64_t time);
	void access_ok (uint64_t time);

	void start_test (void);
	void end_test (void);

	Dcf *m_dcf;
	MacParameters *m_parameters;
	class TestAccessListener *m_listener;
	std::list<uint64_t> m_access_granted_expected;
	bool m_failed;
};

class TestAccessListener : public DcfAccessListener {
public:
	TestAccessListener (DcfTest *test)
		: m_test (test) {}
	virtual ~TestAccessListener () {}
	virtual void access_granted_now (void) {
		m_test->access_granted_now ();
	}
	virtual bool access_needed (void) {
		return m_test->access_needed ();
	}
	virtual bool accessing_and_will_notify (void) {
		return m_test->accessing_and_will_notify ();
	}
private:
	DcfTest *m_test;
};



DcfTest::DcfTest ()
	: Test ("Dcf") {}

void 
DcfTest::access_granted_now (void)
{
	if (m_access_granted_expected.empty ()) {
		failure () << "DCF "
			   << "Failure: unexpected access granted at="<<Simulator::now_us ()
			   << std::endl;
		m_failed = true;
		return;
	}
	uint64_t expected = m_access_granted_expected.front ();
	if (expected != Simulator::now_us ()) {
		failure () << "DCF "
			   << "Failure: access granted at=" << Simulator::now_us ()
			   << ", expected at="<<expected
			   << std::endl;
		m_failed = true;
		return;
	}
	m_access_granted_expected.erase (m_access_granted_expected.begin ());
}
bool 
DcfTest::access_needed (void)
{
	return true;
}
bool 
DcfTest::accessing_and_will_notify (void)
{
	return false;
}

void 
DcfTest::add_rx_ok_evt (uint64_t at, uint64_t duration)
{
	Simulator::schedule_abs_us (at, make_event (&Dcf::notify_rx_start_now, m_dcf, duration));
	Simulator::schedule_abs_us (at+duration, make_event (&Dcf::notify_rx_end_ok_now, m_dcf));
}
void 
DcfTest::add_rx_error_evt (uint64_t at, uint64_t duration)
{
	Simulator::schedule_abs_us (at, make_event (&Dcf::notify_rx_start_now, m_dcf, duration));
	Simulator::schedule_abs_us (at+duration, make_event (&Dcf::notify_rx_end_error_now, m_dcf));
}
void 
DcfTest::add_tx_evt (uint64_t at, uint64_t duration)
{
	Simulator::schedule_abs_us (at, make_event (&Dcf::notify_tx_start_now, m_dcf, duration));
}
void 
DcfTest::add_nav_reset (uint64_t at, uint64_t start, uint64_t duration)
{
	Simulator::schedule_abs_us (at, make_event (&Dcf::notify_nav_reset, m_dcf, start, duration));
}
void 
DcfTest::add_nav_start (uint64_t at, uint64_t start, uint64_t duration)
{
	Simulator::schedule_abs_us (at, make_event (&Dcf::notify_nav_start, m_dcf, start, duration));
}
void 
DcfTest::add_nav_continue (uint64_t at, uint64_t start, uint64_t duration)
{
	Simulator::schedule_abs_us (at, make_event (&Dcf::notify_nav_continue, m_dcf, start, duration));
}
void 
DcfTest::add_access_request (uint64_t time)
{
	Simulator::schedule_abs_us (time, make_event (&Dcf::request_access, m_dcf));
}
void 
DcfTest::add_access_error (uint64_t time)
{
	Simulator::schedule_abs_us (time, make_event (&DcfTest::access_error, this, time));
}
void 
DcfTest::add_access_error_but_ok (uint64_t time)
{
	Simulator::schedule_abs_us (time, make_event (&DcfTest::access_error_but_ok, this, time));
}
void 
DcfTest::add_access_ok (uint64_t time)
{
	Simulator::schedule_abs_us (time, make_event (&DcfTest::access_ok, this, time));
}

void 
DcfTest::access_error (uint64_t time)
{
	m_dcf->update_failed_cw ();
	m_dcf->start_backoff ();
}
void 
DcfTest::access_error_but_ok (uint64_t time)
{
	m_dcf->reset_cw ();
	m_dcf->start_backoff ();
}
void 
DcfTest::access_ok (uint64_t time)
{
	m_dcf->reset_cw ();
	m_dcf->start_backoff ();
}

void 
DcfTest::expect_access_granted (uint64_t time)
{
	m_access_granted_expected.push_back (time);
}

void 
DcfTest::start_test (void)
{
	m_dcf = new Dcf ();
	m_dcf->reset_rng (0);
	m_parameters = new MacParameters ();
	m_listener = new TestAccessListener (this);
	m_dcf->set_parameters (m_parameters);
	m_dcf->register_access_listener (m_listener);	

	m_parameters->set_slot_time_us (1);
	m_dcf->set_difs_us (3);
	m_dcf->set_eifs_us (4);
	m_dcf->set_cw_bounds (8, 64);
}
void 
DcfTest::end_test (void)
{
	if (!m_access_granted_expected.empty ()) {
		failure () << "DCF: access not granted as expected"
			   << std::endl;
	}
	m_access_granted_expected.erase (m_access_granted_expected.begin (),
					 m_access_granted_expected.end ());
	Simulator::destroy ();
	delete m_dcf;
	delete m_parameters;
	delete m_listener;
}

bool
DcfTest::run_tests (void)
{
	m_failed = false;

	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 2+8);
	add_rx_ok_evt (32, 5);
	add_access_request (15);
	add_access_request (16);
	add_access_request (20);
	expect_access_granted (51);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 2+5);
	add_rx_ok_evt (32, 7);
	add_access_request (15);
	add_access_request (16);
	add_access_request (20);
	expect_access_granted (50);
	Simulator::run ();
	end_test ();

	start_test ();
	add_access_request (10);
	expect_access_granted (10);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 2+8);
	add_rx_ok_evt (32, 7);
	add_access_request (40);
	expect_access_granted (43);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 2+8);
	add_rx_ok_evt (32, 7);
	add_access_request (41);
	expect_access_granted (43);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 2+8);
	add_rx_ok_evt (32, 7);
	add_access_request (43);
	expect_access_granted (43);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_error_evt (10, 20);
	add_rx_ok_evt (31, 7);
	add_access_request (39);
	expect_access_granted (41);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_error_evt (10, 20);
	add_rx_error_evt (31, 7);
	add_access_request (39);
	expect_access_granted (42);
	Simulator::run ();
	end_test ();


	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 200);
	add_rx_ok_evt (35, 10);
	add_nav_reset (45, 45, 0);
	add_access_request (32);
	expect_access_granted (48);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 200);
	add_rx_ok_evt (35, 10);
	add_nav_reset (45, 45, 0);
	Simulator::run ();
	end_test ();

	start_test ();
	add_rx_ok_evt (10, 20);
	add_nav_start (30, 30, 200);
	add_rx_ok_evt (35, 10);
	add_nav_reset (45, 45, 0);
	add_access_request (49);
	expect_access_granted (49);
	Simulator::run ();
	end_test ();


	return !m_failed;
}

static DcfTest g_dcf_test;

}; // namespace yans


#endif /* RUN_SELF_TESTS */
