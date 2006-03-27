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
#include "buffer.h"

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
StatusCode::is_success (void)
{
	return (m_code == 0)?true:false;
}

uint32_t 
StatusCode::get_size (void) const
{
	return 2;
}
void 
StatusCode::write_to (Buffer *buffer) const
{
	buffer->write_hton_u16 (m_code);
}
uint32_t 
StatusCode::read_from (Buffer *buffer)
{
	m_code = buffer->read_ntoh_u16 ();
	return 2;
}


}; // namespace yans
