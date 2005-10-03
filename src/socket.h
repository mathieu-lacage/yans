/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
#include "ipv4-address.h"

class Socket {
public:
	bool is_ipv6 (void);
	bool is_ipv4 (void);

	/* binding-related operations. */
	void bind (Ipv4Address address, uint16_t port);
	Ipv4Address get_ipv4_address (void);
	uint16_t get_port (void);

	/* connection-related operations. */
	void connect (Ipv4Address address, uint16_t port);
	bool is_connected (void);
	bool is_connecting (void);
	Ipv4Address get_peer_ipv4_address (void);
	uint16_t get_peer_port (void);

#if 0
	void accept (SocketAddress *addr);
	void listen (int backlog);

	void read (void);
	void write (void);

	void recv (void);

	void recv_from (void);

	uint32_t send (uint8_t const *buf, uint32_t length, int flags);
	uint32_t send_to (uint8_t const *buf, uint32_t length, int flags, const struct sockaddr *to, socklen_t tolen);
	uint32_t send_msg (void);
#endif
	bool is_ok (void);
};

#endif /* SOCKET_H */
