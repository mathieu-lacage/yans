/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IPV4_H
#define IPV4_H

#include "sgi-hashmap.h"
#include <vector>

class NetworkInterface;
class TransportProtocol;
class Packet;

class Ipv4 {
public:
	Ipv4 ();
	~Ipv4 ();

	void set_destination (uint32_t destination);
	void set_protocol (uint8_t protocol);
	void send (Packet *packet);

	/* invoked from higher-layers. */
	void register_network_interface (NetworkInterface *interface);
	void register_transport_protocol (TransportProtocol *protocol);
	/* invoked from lower-layers. */
	void receive (Packet *packet, NetworkInterface *from);

private:
	NetworkInterface *choose_out_interface (uint32_t destination);

	typedef Sgi::hash_map<uint8_t, TransportProtocol *> TransportProtocols;
	typedef std::vector<NetworkInterface *> NetworkInterfaces;
	TransportProtocols m_protocols;
	NetworkInterfaces m_interfaces;
	uint32_t m_send_destination;
	uint8_t m_send_protocol;
};

#endif /* IPV4_H */

