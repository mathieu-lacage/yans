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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

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
