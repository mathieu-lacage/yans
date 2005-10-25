/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#include "transport-protocol.h"
#include "ipv4-address.h"

class Ipv4;
class Ipv4EndPoints;
class Host;

class Udp : public TransportProtocol {
public:
	Udp ();
	virtual ~Udp ();

	void set_host (Host *host);
	void set_ipv4 (Ipv4 *ipv4);

	Ipv4EndPoints *get_end_points (void);

	virtual uint8_t get_protocol (void);
	virtual void receive (Packet *packet);
	void send (Packet *packet);
 private:
	static const uint8_t UDP_PROTOCOL;
	Ipv4 *m_ipv4;
	Host *m_host;
	Ipv4EndPoints *m_end_points;
};

#endif /* UDP_H */
