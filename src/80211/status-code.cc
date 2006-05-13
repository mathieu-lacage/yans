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

#include "status-code.h"

namespace yans {

StatusCode::StatusCode ()
{}
void 
StatusCode::set_success (void)
{
	m_code = 0;
}
void 
StatusCode::set_failure (void)
{
	m_code = 1;
}

bool 
StatusCode::is_success (void) const
{
	return (m_code == 0)?true:false;
}
uint16_t 
StatusCode::peek_code (void) const
{
	return m_code;
}
void 
StatusCode::set_code (uint16_t code)
{
	m_code = code;
}

uint32_t 
StatusCode::get_size (void) const
{
	return 2;
}

}; // namespace yans
