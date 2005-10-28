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

#include "io-runnable.h"
#include "fiber-scheduler.h"
#include <sys/poll.h>

IoJob::IoJob (int fd)
	: m_fd (fd)
{}

IoJob::~IoJob ()
{}

int
IoJob::get_fd (void)
{
	return m_fd;
}

IoRunnable::IoRunnable ()
	: m_run (true),
	  m_wait (0)
{}

void 
IoRunnable::add_read (IoJob *job)
{
	m_reads.push_back (job);
	increment_poll_fds ();
	notify_job_added ();
}
void 
IoRunnable::add_write (IoJob *job)
{
	m_writes.push_back (job);
	increment_poll_fds ();
	notify_job_added ();
}

void
IoRunnable::remove_read (IoJob *job)
{
	m_reads.remove (job);
	decrement_poll_fds ();
}
void
IoRunnable::remove_write (IoJob *job)
{
	m_writes.remove (job);
	decrement_poll_fds ();
}

void 
IoRunnable::stop (void)
{
	m_run = false;
}

void 
IoRunnable::run (void)
{
	while (m_run) {
		wait_for_jobs_or_yield ();
		uint32_t n = 0;
		for (JobsI i = m_reads.begin (); i != m_reads.end (); i++) {
			m_poll_fds[n].fd = (*i)->get_fd ();
			m_poll_fds[n].events = POLLIN;
			n++;
		}
		for (JobsI j = m_writes.begin (); j != m_writes.end (); j++) {
			m_poll_fds[n].fd = (*j)->get_fd ();
			m_poll_fds[n].events = POLLOUT;
			n++;
		}
		int retval = poll (m_poll_fds, m_n_used_poll_fds, 0);
		if (retval == -1) {
			/* error occured. 
			 */
			assert (false);
		} else if (retval > 0) {
			int k = 0;
			for (JobsI l = m_reads.begin (); l != m_reads.end () && k < retval; l++) {
				if (m_poll_fds[k].revents) {
					assert (m_poll_fds[k].revents == POLLIN);
					(*l)->notify ();
				}
				k++;
			}
			for (JobsI m = m_writes.begin (); m != m_writes.end (); m++) {
				if (m_poll_fds[k].revents) {
					assert (m_poll_fds[k].revents == POLLOUT);
					(*m)->notify ();
				}
			}
		}
	}
}

void
IoRunnable::wait_for_jobs_or_yield (void)
{
	if (m_reads.empty () && 
	    m_writes.empty ()) {
		m_wait.down ();
	} else {
		FiberScheduler::instance ()->schedule ();
	}
}
void
IoRunnable::notify_job_added (void)
{
	if (m_reads.empty () && 
	    m_writes.empty ()) {
		m_wait.up ();
	}
}

void
IoRunnable::increment_poll_fds (void)
{
	if (m_n_used_poll_fds == m_n_available_poll_fds) {
		m_n_available_poll_fds *= 2;
		struct pollfd *poll_fds = new struct pollfd [m_n_available_poll_fds];
		delete [] m_poll_fds;
		m_poll_fds = poll_fds;
	}
	m_n_used_poll_fds++;
}
void
IoRunnable::decrement_poll_fds (void)
{
	m_n_used_poll_fds--;
	if (m_n_used_poll_fds <= 8) {
		return;
	}
	if (m_n_used_poll_fds == (m_n_available_poll_fds / 2)) {
		m_n_available_poll_fds /= 2;
		struct pollfd *poll_fds = new struct pollfd [m_n_available_poll_fds];
		delete [] m_poll_fds;
		m_poll_fds = poll_fds;
	}
}
