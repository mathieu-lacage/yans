/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CABLE_H
#define CABLE_H

#include <list>

class Packet;
class EthernetNetworkInterface;

class Cable {
public:
	Cable ();

	void connect_to (EthernetNetworkInterface *interface);

	void send (Packet *packet, EthernetNetworkInterface *sender);

private:
	typedef std::list<EthernetNetworkInterface *> EthernetNetworkInterfaces;
	typedef std::list<EthernetNetworkInterface *>::iterator EthernetNetworkInterfacesI;

	EthernetNetworkInterfaces m_interfaces;
};


#endif /* CABLE_H */
