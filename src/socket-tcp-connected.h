/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SOCKET_TCP_CONNECTED_H
#define SOCKET_TCP_CONNECTED_H

#include <stdint.h>
#include "ipv4-address.h"

class SocketTcpConnected {
 public:
	Ipv4Address get_peer_ipv4_address (void);
	uint16_t get_peer_port (void);
	
	uint32_t send (uint8_t const *buf, uint32_t length, int flags);
	uint32_t recv (uint8_t *buf, uint32_t length, int flags);
};

#endif /* SOCKET_TCP_CONNECTED_H */
