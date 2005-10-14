/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "cable.h"
#include "packet.h"
#include "ethernet-network-interface.h"

Cable::Cable ()
{}

void 
Cable::connect_to (EthernetNetworkInterface *interface)
{
	m_interfaces.push_back (interface);
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
