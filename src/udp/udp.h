/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#include "transport-protocol.h"
#include "ipv4-address.h"

class Ipv4;

class Udp : public TransportProtocol {
public:
	virtual ~Udp ();

	void set_ipv4 (Ipv4 *ipv4);

	void send (Packet *packet);

	virtual uint8_t get_protocol (void);
	virtual void receive (Packet *packet);

 private:
	static const uint8_t UDP_PROTOCOL;
	Ipv4 *m_ipv4;
};

#endif /* UDP_H */
