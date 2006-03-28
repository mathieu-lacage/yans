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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "timeout.h"
#include "simulator.h"
#include "event.tcc"

namespace yans {

Timeout::Timeout (uint64_t interval_us, uint32_t count)
	: m_interval_us (interval_us),
	  m_count (count),
	  m_current_count (count),
	  m_stop (false),
	  m_callback (0)
{}
Timeout::~Timeout ()
{
	delete m_callback;
}

void Timeout::start (ExpireCallback *callback)
{
	Simulator::insert_in_us (m_interval_us,
				 make_event (&Timeout::expire, this));
	m_callback = callback;
}
void Timeout::stop (void)
{
	m_stop = true;
	delete m_callback;
}
void Timeout::restart (void)
{
	m_current_count = m_count;
}
void
Timeout::expire (void)
{
	if (m_stop) {
		return;
	}
	m_current_count--;
	if (m_current_count == 0) {
		(*m_callback) ();
		return;
	}
	Simulator::insert_in_us (m_interval_us,
				 make_event (&Timeout::expire, this));
}


}; // namespace yans
