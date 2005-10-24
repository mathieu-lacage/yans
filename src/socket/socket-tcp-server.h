/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SOCKET_TCP_SERVER_H
#define SOCKET_TCP_SERVER_H

#include <stdint.h>
#include "ipv4-address.h"

class SocketTcpConnected;

SocketTcpServer {
 public:
	SocketTcpServer (void);

	void bind (Ipv4Address address, uint16_t port);
	Ipv4Address get_ipv4_address (void);
	uint16_t get_port (void);

	SocketTcpConnected *accept (void);
};

#endif /* SOCKET_TCP_SERVER_H */
