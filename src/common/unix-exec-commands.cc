/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "exec-commands.h"
#include "callback.h"
#include <cassert>
#include <vector>

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define noTRACE_COMMAND 1

#ifdef TRACE_COMMAND
#include <iostream>
# define TRACE(x) \
std::cout << "COMMAND TRACE " << x << std::endl;
#else /* TRACE_COMMAND */
# define TRACE(format,...)
#endif /* TRACE_COMMAND */


namespace yans {

/* A small Os-independent thread class
 */
class UnixSystemThread {
public:
	UnixSystemThread ();
	virtual ~UnixSystemThread () = 0;
private:
	static void *pthread_run (void *thread);
	virtual void real_run (void) = 0;

	pthread_t m_system_thread;
};

UnixSystemThread::UnixSystemThread ()
{
	int retval = pthread_create (&m_system_thread,
				     NULL,
				     UnixSystemThread::pthread_run,
				     this);
	assert (retval == 0);
}
UnixSystemThread::~UnixSystemThread ()
{}
void *
UnixSystemThread::pthread_run (void *thread)
{
	UnixSystemThread *self = reinterpret_cast<UnixSystemThread *> (thread);
	self->real_run ();
	return thread;
}

/* A small Os-independent semaphore class
 */
class UnixSystemSemaphore {
public:
	UnixSystemSemaphore (uint32_t init);
	void post (void);
	void post (uint32_t n);
	void wait (void);
	void wait (uint32_t n);
private:
	sem_t m_sem;
};

UnixSystemSemaphore::UnixSystemSemaphore (uint32_t init)
{
	sem_init (&m_sem, 0, init);
}
void 
UnixSystemSemaphore::post (void)
{
	sem_post (&m_sem);
}
void 
UnixSystemSemaphore::wait (void)
{
	sem_wait (&m_sem);
}
void 
UnixSystemSemaphore::post (uint32_t n)
{
	for (uint32_t i = 0; i < n; i++) {
		post ();
	}
}
void 
UnixSystemSemaphore::wait (uint32_t n)
{
	for (uint32_t i = 0; i < n; i++) {
		wait ();
	}
}

class UnixSystemMutex {
public:
	UnixSystemMutex ();
	~UnixSystemMutex ();
	void lock (void);
	void unlock (void);
private:
	pthread_mutex_t m_mutex;
};

UnixSystemMutex::UnixSystemMutex ()
{
	int retval = pthread_mutex_init (&m_mutex, NULL);
	assert (retval == 0);
}

UnixSystemMutex::~UnixSystemMutex ()
{
	int retval = pthread_mutex_destroy (&m_mutex);
	assert (retval == 0);
}

void
UnixSystemMutex::lock (void)
{
	pthread_mutex_lock (&m_mutex);
}

void
UnixSystemMutex::unlock (void)
{
	pthread_mutex_unlock (&m_mutex);
}




class CommandSystemThread : public UnixSystemThread {
public:
	typedef Callback<void, CommandSystemThread *> DoneCallback;
	CommandSystemThread ();
	void start (Command command, ExecCommands::CommandCallback callback, DoneCallback callback);
	void stop (void);
private:
	virtual void real_run (void);
	void read_data (int fd);
	UnixSystemSemaphore m_sem;
	bool m_stop;
	Command m_command;
	ExecCommands::CommandCallback m_callback;
	DoneCallback m_done;
};

CommandSystemThread::CommandSystemThread ()
	: m_sem (0),
	  m_stop (false)
{}

void
CommandSystemThread::start (Command command, ExecCommands::CommandCallback callback, DoneCallback done)
{
	m_command = command;
	m_callback = callback;
	m_done = done;
	m_sem.post ();
}

void
CommandSystemThread::stop (void)
{
	m_stop = true;
	m_sem.post ();
}
void
CommandSystemThread::read_data (int fd)
{
	char buffer[512];
	TRACE ("start read");
	int retval = read (fd, buffer, 512);
	while (retval > 0) {
		m_callback (buffer, retval);
		TRACE ("next read" << retval);
		retval = read (fd, buffer, 512);
	}
	TRACE ("read done");
}

void
CommandSystemThread::real_run (void)
{
	while (true) {
		m_sem.wait ();
		if (m_stop) {
			break;
		}
		int retval;
		int filedes[2];
		retval = pipe (filedes);
		assert (retval == 0);
		pid_t pid = fork ();
		if (pid == -1) {
			assert (false);
			// error in parent fork.
		} else if (pid == 0) {
			// success, child.
			dup2 (1, filedes[0]);
			close (filedes[0]);
			close (filedes[1]);
			char ** args = (char **)malloc (sizeof (char *) * (m_command.get_n () - 1));
			char const *file = m_command.get (0);
			for (uint32_t i = 1; i < (m_command.get_n () - 1); i++) {
				args[i-1] = strdup (m_command.get (i));
			}
			retval = execvp (file, args);
			// NOTREACHED
			assert (false);
		} else {
			// success, parent.
			close (filedes[1]);
			read_data (filedes[0]);
		}
		m_done (this);
	}
}

class ExecCommandsPrivate {
public:
	ExecCommandsPrivate (uint32_t pool_size);
	~ExecCommandsPrivate ();
	void add (Command command, ExecCommands::CommandCallback callback);
	void start_and_wait (void);
private:
	struct CommandRequest {
		Command m_command;
		ExecCommands::CommandCallback m_callback;
	};
	typedef std::vector<CommandSystemThread *> Threads;
	typedef std::vector<CommandSystemThread *>::iterator ThreadsI;
	typedef std::vector<CommandRequest> Requests;
	typedef std::vector<CommandRequest>::iterator RequestsI;
	void command_done (CommandSystemThread *thread);

	Requests m_requests;
	UnixSystemSemaphore m_n_threads;
	uint32_t m_pool_size;
	UnixSystemMutex m_threads_mutex;
	Threads m_threads;
};


ExecCommandsPrivate::ExecCommandsPrivate (uint32_t pool_size)
	: m_n_threads (0),
	  m_pool_size (pool_size)
{
	m_threads_mutex.lock ();
	for (uint32_t i = 0; i < pool_size; i++) {
		m_threads.push_back (new CommandSystemThread ());
	}
	m_threads_mutex.unlock ();
	m_n_threads.post (m_pool_size);
}

ExecCommandsPrivate::~ExecCommandsPrivate ()
{
	start_and_wait ();
	m_threads_mutex.lock ();
	for (ThreadsI i = m_threads.begin (); i != m_threads.end (); i++) {
		(*i)->stop ();
		delete *i;
	}
	m_threads.erase (m_threads.begin (), m_threads.end ());
	m_threads_mutex.unlock ();
}


void
ExecCommandsPrivate::add (Command command, ExecCommands::CommandCallback callback)
{
	struct CommandRequest request;
	request.m_command = command;
	request.m_callback = callback;
	m_requests.push_back (request);
}
void 
ExecCommandsPrivate::start_and_wait (void)
{
	for (RequestsI i = m_requests.begin (); i != m_requests.end (); i++) {
		TRACE ("wait for thread");
		m_n_threads.wait ();
		m_threads_mutex.lock ();
		CommandSystemThread *thread = m_threads.back ();
		m_threads.pop_back ();
		m_threads_mutex.unlock ();
		TRACE ("start command");
		thread->start (i->m_command, i->m_callback, make_callback (&ExecCommandsPrivate::command_done, this));
	}
	TRACE ("wait end of requests");
	m_n_threads.wait (m_pool_size);
	TRACE ("completed run");
}
void
ExecCommandsPrivate::command_done (CommandSystemThread *thread)
{
	TRACE ("command done");
	m_threads_mutex.lock ();
	m_threads.push_back (thread);
	m_threads_mutex.unlock ();
	m_n_threads.post ();
	TRACE ("notify command done");
}
				   


void 
Command::append (std::string arg)
{
	m_args.push_back (arg);
}


uint32_t
Command::get_n (void)
{
	return m_args.size ();
}
char const*
Command::get (uint32_t i)
{
	return m_args[i].c_str ();
}



ExecCommands::ExecCommands (uint32_t pool_size)
	: m_priv (new ExecCommandsPrivate (pool_size))
{
	assert (pool_size != 0);
}
void 
ExecCommands::add (Command command, CommandCallback callback)
{
	m_priv->add (command, callback);
}
void 
ExecCommands::start_and_wait (void)
{
	m_priv->start_and_wait ();
}

}; // namespace yans

#ifdef RUN_SELF_TESTS

#include "test.h"
#include <iomanip>
#include <iostream>

namespace yans {

class ExecCommandsTest: public Test {
public:
	ExecCommandsTest ();
	virtual ~ExecCommandsTest ();
	virtual bool run_tests (void);
private:
	void command_output (char const *data, uint32_t size);
};

ExecCommandsTest::ExecCommandsTest ()
	: Test ("ExecCommands")
{}
ExecCommandsTest::~ExecCommandsTest ()
{}
void 
ExecCommandsTest::command_output (char const *data, uint32_t size)
{
	for (uint32_t i = 0; i < size; i++) {
		std::cout << data[i];
	};
}
bool 
ExecCommandsTest::run_tests (void)
{
	bool ok = true;
	ExecCommands commands = ExecCommands (1);
	Command command;
	command.append ("ls");
	commands.add (command, make_callback (&ExecCommandsTest::command_output, this));
	commands.start_and_wait ();
	
	return ok;
}

static ExecCommandsTest g_exec_commands_test;

#endif /* RUN_SELF_TESTS */


}; // namespace yans

