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
#ifndef START_REMOTE_CONTEXTS_H
#define START_REMOTE_CONTEXTS_H

#include "yans/system-thread.h"
#include "yans/system-semaphore.h"
#include "yans/callback.h"
#include "registry.h"

class StartRemoteContexts : public yans::SystemThread {
public:
	StartRemoteContexts (char const *registry_ref, Registry_ptr registry, char const *filename);
	virtual ~StartRemoteContexts ();
	void registered (void);
	void set_started_callback (yans::Callback<void> cb);
private:
	virtual void real_run (void);
	std::string m_name;
	yans::SystemSemaphore m_wait_started;
	Registry_ptr m_registry;
	yans::Callback<void> m_started_cb;
};


#endif /* START_REMOTE_CONTEXTS_H */
