/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IO_RUNNABLE_H
#define IO_RUNNABLE_H

#include "runnable.h"
#include "semaphore.h"
#include <list>

struct pollfd;

class IoJob {
public:
	IoJob (int fd);
	virtual ~IoJob () = 0;
	int get_fd (void);

	virtual void notify (void) = 0;
private:
	int m_fd;
};

class IoRunnable : public Runnable {
public:
	IoRunnable ();

	void stop (void);

	void add_read  (IoJob *job);
	void add_write (IoJob *job);

	void remove_write (IoJob *job);
	void remove_read  (IoJob *job);
private:
	virtual void run (void);

	void wait_for_jobs_or_yield (void);
	void notify_job_added (void);
	void increment_poll_fds (void);
	void decrement_poll_fds (void);
	
	typedef std::list<IoJob *> Jobs;
	typedef std::list<IoJob *>::iterator JobsI;

	Jobs m_reads;
	Jobs m_writes;
	struct pollfd *m_poll_fds;
	uint32_t m_n_available_poll_fds;
	uint32_t m_n_used_poll_fds;
	bool m_run;
	Semaphore m_wait;
};


#endif /* IO_RUNNABLE_H */
