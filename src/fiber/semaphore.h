/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include <list>

class Fiber;

class Semaphore {
public:
	Semaphore (int32_t n);
	~Semaphore ();

	void up (void);
	void down (void);

	void up (uint8_t delta);
	void down (uint8_t delta);
private:
	int32_t m_n;
	std::list<Fiber *> m_waiting;
};


#endif /* SEMAPHORE_H */
