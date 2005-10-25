/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SOCKET_TCP_CLIENT_H
#define SOCKET_TCP_CLIENT_H

#include <stdint.h>
#include "ipv4-address.h"

SocketTcpClient {
 public:
	SocketTcpClient (void);

	void bind (Ipv4Address address, uint16_t port);
	Ipv4Address get_ipv4_address (void);
	uint16_t get_port (void);

	SocketTcpConnected *connect (Ipv4Address address, uint16_t port);
	bool is_connected (void);
	bool is_connecting (void);
	SocketTcpConnected *get_connected (void);
};

#endif /* SOCKET_TCP_CLIENT_H */
