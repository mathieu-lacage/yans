/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "time.h"

#include <sys/time.h>

void 
Time::start (void)
{
	struct timezone tz;
	gettimeofday (&m_start_tv, &tz);
}

unsigned long long 
Time::end (void)
{
	struct timezone tz;
	gettimeofday (&m_end_tv, &tz);
	unsigned long long end = m_end_tv.tv_sec *1000 + m_end_tv.tv_usec / 1000;
	unsigned long long start = m_start_tv.tv_sec *1000 + m_start_tv.tv_usec / 1000;
	return end - start;
}
