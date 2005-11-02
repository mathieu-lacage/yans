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

#include "cable.h"
#include "packet.h"
#include "ethernet-network-interface.h"

Cable::Cable ()
	: m_ref (1)
{}

void
Cable::ref (void)
{
	m_ref++;
}
void
Cable::unref (void)
{
	m_ref--;
	if (m_ref == 0) {
		delete this;
	}
}

void 
Cable::connect_to (EthernetNetworkInterface *interface)
{
	m_interfaces.push_back (interface);
	interface->connect_to (this);
}

void 
Cable::send (Packet *packet, EthernetNetworkInterface *sender)
{
	for (EthernetNetworkInterfacesI i = m_interfaces.begin ();
	     i != m_interfaces.end (); i++) {
		if ((*i) != sender) {
			(*i)->recv (packet->copy ());
		}
	}
	packet->unref ();
}
