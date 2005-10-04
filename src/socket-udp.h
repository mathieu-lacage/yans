/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#include <stdint.h>
#include "ipv4-address.h"

class Host;

class SocketUdp {
 public:
	SocketUdp (Host *host);

	void close (void);

	/* binding-related operations. */
	void bind (Ipv4Address address, uint16_t port);
	Ipv4Address get_ipv4_address (void);
	uint16_t get_port (void);

	void set_peer (Ipv4Address address, uint16_t port);

	uint32_t send (uint8_t const *buf, uint32_t length, int flags);
	uint32_t recv (uint8_t const *buf, uint32_t length, int flags);
 private:
	Host *m_host;
	Ipv4Address m_self_address;
	uint16_t m_self_port;
	Ipv4Address m_peer_address;
	uint16_t m_peer_port;
};

#endif /* SOCKET_UDP */
