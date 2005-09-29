/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IPV4_H
#define IPV4_H

#include "sgi-hashmap.h"
#include <vector>

#include "ipv4-address.h"

class NetworkInterface;
class TransportProtocol;
class Packet;
class Ipv4Route;

class Ipv4 {
public:
	Ipv4 ();
	~Ipv4 ();

	void set_routing_table (Ipv4Route *route);

	void set_destination (Ipv4Address destination);
	void set_protocol (uint8_t protocol);
	void send (Packet *packet);

	/* invoked from higher-layers. */
	void register_transport_protocol (TransportProtocol *protocol);
	/* invoked from lower-layers. */
	void receive (Packet *packet, NetworkInterface *from);

private:
	Ipv4Route *m_route;
	typedef Sgi::hash_map<uint8_t, TransportProtocol *> TransportProtocols;
	typedef std::vector<NetworkInterface *> NetworkInterfaces;
	typedef std::vector<NetworkInterface *>::const_iterator NetworkInterfacesCI;
	TransportProtocols m_protocols;
	NetworkInterfaces m_interfaces;
	Ipv4Address m_send_destination;
	uint8_t m_send_protocol;
};

#endif /* IPV4_H */

