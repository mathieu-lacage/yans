/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CABLE_H
#define CABLE_H

#include <list>

class Packet;
class EthernetNetworkInterface;

class Cable {
public:
	Cable ();

	void ref (void);
	void unref (void);

	void connect_to (EthernetNetworkInterface *interface);

	void send (Packet *packet, EthernetNetworkInterface *sender);

private:
	typedef std::list<EthernetNetworkInterface *> EthernetNetworkInterfaces;
	typedef std::list<EthernetNetworkInterface *>::iterator EthernetNetworkInterfacesI;

	EthernetNetworkInterfaces m_interfaces;
	uint32_t m_ref;
};


#endif /* CABLE_H */
