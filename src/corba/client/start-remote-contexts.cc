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
#include "start-remote-contexts.h"
#include "yans/system-thread.h"
#include "yans/system-semaphore.h"
#include "yans/exec-commands.h"

StartRemoteContexts::StartRemoteContexts (char const *name, Registry_ptr registry, char const *filename)
	: m_name (name),
	  m_wait_started (0),
	  m_registry (registry)
{}
StartRemoteContexts::~StartRemoteContexts ()
{}

void 
StartRemoteContexts::set_started_callback (yans::Callback<void> cb)
{
	m_started_cb = cb;
}

void
StartRemoteContexts::registered (void)
{
	m_wait_started.post ();
}

void
StartRemoteContexts::real_run (void)
{
	std::string registry_str = std::string ("--registry=");
	registry_str.append (m_name);
	yans::ExecCommands commands = yans::ExecCommands (1);
	yans::Command a;
	yans::Command b;
	a.append ("./build-dir/bin/context-server");
	a.append ("--name=a");
	a.append (registry_str);
	commands.add (a, "a");
	b.append ("./build-dir/bin/context-server");
	b.append ("--name=b");
	b.append (registry_str);
	commands.add (b, "b");

	commands.enable_log ("main.log");
	commands.start ();
	m_wait_started.wait (2);
	m_started_cb ();
}
