/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

class Clock {
public:
	static Clock *instance (void);

	/* in microseconds */
	uint64_t get_current_us (void);
	void update_current_us (uint64_t new_time);

	/* in seconds */
	double get_current_s (void);
	void update_current_s (double new_time);
private:
	Clock ();
	uint64_t m_current_us;
	static Clock *m_instance;
};


#endif /* CLOCK_H */
