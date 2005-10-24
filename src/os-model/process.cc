/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <string.h>

#include "fiber.h"
#include "process.h"



Process::Process (char const *name)
	: Thread (name),
	  m_argc (1),
	  m_retval (-1)
{
	m_argv = new char *[1];
	m_argv[0] = strdup (name);
}
Process::Process (Host *host, char const *name)
	: Thread (host, name),
	  m_argc (1),
	  m_retval (-1)
{
	m_argv = new char *[1];
	m_argv[0] = strdup (name);
}
Process::Process (int argc, char const *argv[])
	: Thread (argv[0]),
	  m_argc (argc),
	  m_retval (-1) 
{
	assert (argc > 0);
	m_argv = new char *[argc];
	for (int i = 0; i < argc; i++) {
		m_argv[i] = strdup (argv[i]);
	}
}
Process::Process (Host *host, int argc, char const *argv[])
	: Thread (host, argv[0]),
	  m_argc (argc),
	  m_retval (-1) 
{
	assert (argc > 0);
	m_argv = new char *[argc];
	for (int i = 0; i < argc; i++) {
		m_argv[i] = strdup (argv[i]);
	}
}
Process::~Process ()
{
	m_retval = 0xdeadbeaf;
	for (int i = 0; i < m_argc; i++) {
		free (m_argv[i]);
		m_argv[i] = (char *)0xdeadbeaf;
	}
	delete [] m_argv;
	m_argc = 0xdeadbeaf;
	m_argv = (char **)0xdeadbeaf;
}

void
Process::run (void)
{
	m_retval = run (m_argc, m_argv);
}


int
Process::get_retval (void)
{
	return m_retval;
}
