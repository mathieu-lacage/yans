/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

class Clock {
public:
	Clock ();
	/* in microseconds */
	uint64_t get_current_us (void);
	void update_current_us (uint64_t new_time);

	/* in seconds */
	double get_current_s (void);
private:
	uint64_t m_current_us;
};


#endif /* CLOCK_H */
