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
#include "ipv4-network-interface.h"

namespace yapns {

Ipv4NetworkInterface::Ipv4NetworkInterface (::Remote::Ipv4NetworkInterface_ptr remote)
{
	m_remote = ::Remote::Ipv4NetworkInterface::_duplicate (remote);
}
Ipv4NetworkInterface::~Ipv4NetworkInterface ()
{
	CORBA::release (m_remote);
}

::Remote::Ipv4NetworkInterface_ptr 
Ipv4NetworkInterface::peek_remote (void)
{
	return m_remote;
}

}; // namespace yapns
