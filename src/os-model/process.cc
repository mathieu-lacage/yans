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
