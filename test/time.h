/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef TIME_H
#define TIME_H

#include <sys/time.h>

class Time {
public:
	void start (void);
	unsigned long long end (void);
private:
	struct timeval m_start_tv;
	struct timeval m_end_tv;
};

#endif /* TIME_H */
