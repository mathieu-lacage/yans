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

#include "scheduler.h"
#include "packet.h"

#include "simple-net-interface.h"

class Delay : public Handler {
public:
	Delay (SimpleNetInterface *interface, double delay) 
		: m_interface (interface),
		  m_delay (delay) 
	{}
	
	void start (Packet *packet) {
		Scheduler::instance ().schedule (this, packet, m_delay);
	}
	
	virtual void handle(Event* event) {
		Packet *packet = static_cast <Packet *> (event);
		m_interface->reallySendDown (packet);
	}
private:
	SimpleNetInterface *m_interface;
	double m_delay;
};


SimpleNetInterface::SimpleNetInterface (double delay)
	: NetInterface ()
	  
{
	m_delay = new Delay (this, delay);
}
SimpleNetInterface::~SimpleNetInterface ()
{}

void 
SimpleNetInterface::reallySendDown (Packet *packet)
{
	sendDownToChannel (packet);
}

void 
SimpleNetInterface::sendDown (Packet *packet)
{
	m_delay->start (packet);
}
void 
SimpleNetInterface::sendUp (Packet *packet)
{
	sendUpToNode (packet);
}




SimpleNetInterfaceConstructor::SimpleNetInterfaceConstructor ()
{}
SimpleNetInterfaceConstructor::~SimpleNetInterfaceConstructor ()
{}

NetInterface *
SimpleNetInterfaceConstructor::createInterface (void)
{
	return new SimpleNetInterface (1e-3);
}
