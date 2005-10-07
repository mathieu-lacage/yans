/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "clock.h"

#include <cassert>
#include <math.h>

Clock *Clock::m_instance = 0;

Clock *
Clock::instance (void)
{
	if (m_instance == 0) {
		m_instance = new Clock ();
	}
	return m_instance;
}

Clock::Clock ()
	: m_current_us (0)
{}


	/* in microseconds */
uint64_t 
Clock::get_current_us (void)
{
	return m_current_us;
}
void 
Clock::update_current_us (uint64_t new_time)
{
	assert (new_time > 0);
	assert (new_time > m_current_us);
	m_current_us = new_time;
}

double 
Clock::get_current_s (void)
{
	double s = ((double)m_current_us)/1000000;
	return s;
}
void 
Clock::update_current_s (double new_time)
{
	assert (new_time > 0);
	long int us = lrint (new_time * 1000000);
	assert (us > 0);
	update_current_us ((uint64_t)us);
}
