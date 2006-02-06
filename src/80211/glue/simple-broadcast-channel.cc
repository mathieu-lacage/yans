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

#include <list>

#include "packet.h"

#include "simple-broadcast-channel.h"
#include "net-interface.h"


SimpleBroadcastChannel::SimpleBroadcastChannel ()
{}
SimpleBroadcastChannel::~SimpleBroadcastChannel ()
{}

void 
SimpleBroadcastChannel::sendDown (Packet *packet, NetInterface *caller)
{
	std::list<NetInterface *>::iterator tmp;
	bool needToCopy = false;
	/* We could add a simple propagation delay to each packet 
	 * being received here.
	 */
	for (tmp = m_interfaces.begin (); tmp != m_interfaces.end (); tmp++) {
		if ((*tmp) != caller) {
			Packet *p;
			if (needToCopy) {
				p = packet->copy ();
			} else {
				p = packet;
				needToCopy = true;
			}
			(*tmp)->sendUp (p);
		}
	}
}

void 
SimpleBroadcastChannel::registerInterface (NetInterface *interface)
{
	m_interfaces.push_back (interface);
}

