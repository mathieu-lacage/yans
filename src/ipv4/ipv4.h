/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IPV4_H
#define IPV4_H

#include <vector>

#include "ipv4-address.h"

class NetworkInterface;
class TransportProtocol;
class Packet;
class Ipv4Route;
class Host;

class Ipv4 {
public:
	Ipv4 ();
	~Ipv4 ();

	void set_host (Host *host);

	void set_protocol (uint8_t protocol);
	void send (Packet *packet);

	/* invoked from higher-layers. */
	void register_transport_protocol (TransportProtocol *protocol);
	/* invoked from lower-layers. */
	void receive (Packet *packet, NetworkInterface *from);

private:
	Ipv4Route *get_route (void);
	void drop_packet (Packet *packet);
	TransportProtocol *lookup_protocol (uint8_t id);

	Host *m_host;
	typedef std::vector<std::pair<uint8_t, TransportProtocol *> > Protocols;
	typedef std::vector<std::pair<uint8_t, TransportProtocol *> >::iterator ProtocolsI;
	Protocols m_protocols;
	Ipv4Address m_send_destination;
	uint8_t m_send_protocol;
};

#endif /* IPV4_H */

