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
#include "event.h"
#include "simulator.h"

namespace yans {

const double Cable::SPEED_OF_LIGHT = 300000000;

Cable::Cable ()
	: m_ref (this),
	  m_length (100.0),
	  m_bandwidth (1000000)
{}

Cable::Cable (double length, double bandwidth)
	: m_ref (this),
	  m_length (length),
	  m_bandwidth (bandwidth)
{}

void
Cable::ref (void)
{
	m_ref.ref ();
}
void
Cable::unref (void)
{
	m_ref.unref ();
}

void 
Cable::connect_to (EthernetNetworkInterface *a,
		   EthernetNetworkInterface *b)
{
	m_a = a;
	m_b = b;
	a->connect_to (this);
	b->connect_to (this);
}

void
Cable::recv (Packet *packet, EthernetNetworkInterface *to)
{
	to->recv (packet);
	packet->unref ();
}

void 
Cable::send (Packet *packet, EthernetNetworkInterface *sender)
{
	double delay = packet->get_size () * 8 / m_bandwidth + m_length / SPEED_OF_LIGHT;
	EthernetNetworkInterface *rx;
	if (sender == m_a) {
		rx = m_b;
	} else {
		assert (sender == m_b);
		rx = m_a;
	}
	packet->ref ();
	Simulator::insert_in_s (delay, make_event (&Cable::recv, this, packet, rx));
}

}; // namespace yans
