/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#include "transport-protocol.h"
#include "ipv4-address.h"

class Udp : public TransportProtocol {
public:
	virtual ~Udp ();

	void set_destination (Ipv4Address dest);
	void set_destination (uint16_t port);
	void set_source (uint16_t port);

	void send (Packet *packet);

	virtual uint8_t get_protocol (void);
	virtual void receive (Packet *packet);

 private:
	static const uint8_t UDP_PROTOCOL;
	Ipv4Address m_destination;
	uint16_t m_destination_port;
	uint16_t m_source_port;
};

#endif /* UDP_H */
