/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CONTEXT_SIMULATOR_H
#define CONTEXT_SIMULATOR_H

#include "yans/system-thread.h"
#include "yans/system-semaphore.h"
#include "yans/system-mutex.h"
#include "yans/callback.h"

class ContextSimulator : public yans::SystemThread {
public:
	ContextSimulator ();
	void stop_at_us (uint64_t at_us);
	void start (yans::Callback<void> stopped);
	void really_finish (void);
private:
	virtual void real_run (void);
	bool m_finish;
	bool m_running;
	yans::SystemSemaphore m_run;
	yans::Callback<void> m_stopped;
};


#endif /* CONTEXT_SIMULATOR_H */
