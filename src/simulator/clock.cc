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

#include "clock.h"

#include <cassert>

namespace yans {

Clock::Clock ()
	: m_current_us (0)
{}


	/* in microseconds */
uint64_t 
Clock::get_current_us (void)
{
	return m_current_us;
}
void 
Clock::update_current_us (uint64_t new_time)
{
	assert (new_time >= m_current_us);
	m_current_us = new_time;
	m_current_s = ((double)m_current_us)/1000000;
}

double 
Clock::get_current_s (void)
{
	return m_current_s;
}


}; // namespace yans
