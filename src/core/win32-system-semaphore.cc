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

#include "system-semaphore.h"

#include <cassert>
#include <iostream>

namespace yans {

class SystemSemaphorePrivate {
public:
	SystemSemaphorePrivate (uint32_t init);
	void post (void);
	void post (uint32_t n);
	void wait (void);
	void wait (uint32_t n);
private:
};

SystemSemaphorePrivate::SystemSemaphorePrivate (uint32_t init)
{
}
void 
SystemSemaphorePrivate::post (void)
{
}
void 
SystemSemaphorePrivate::wait (void)
{
}
void 
SystemSemaphorePrivate::post (uint32_t n)
{
}
void 
SystemSemaphorePrivate::wait (uint32_t n)
{
}

SystemSemaphore::SystemSemaphore (uint32_t init)
	: m_priv (new SystemSemaphorePrivate (init))
{}
SystemSemaphore::~SystemSemaphore ()
{
	delete m_priv;
}
void 
SystemSemaphore::post (void)
{
	m_priv->post ();
}
void 
SystemSemaphore::post (uint32_t n)
{
	m_priv->post (n);
}
void 
SystemSemaphore::wait (void)
{
	m_priv->wait ();
}
void 
SystemSemaphore::wait (uint32_t n)
{
	m_priv->wait (n);
}



}; // namespace yans
