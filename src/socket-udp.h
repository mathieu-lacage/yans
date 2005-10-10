/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#include <stdint.h>
#include "ipv4-address.h"

class Udp;
class Host;
class Chunk;

class SocketUdp {
 public:
	void enable_non_blocking (void);
	void enable_blocking (void);

	void close (void);

	/* binding-related operations. */
	void bind (Ipv4Address address, uint16_t port);
	Ipv4Address get_ipv4_address (void);
	uint16_t get_port (void);

	void set_peer (Ipv4Address address, uint16_t port);

	uint32_t send (Chunk *data);
	Chunk *recv (void);
 private:
	friend class SocketUdpPacketDestroyNotifier;
	friend class Host;

	SocketUdp (Host *host, Udp *udp);
	void notify_packet_destroyed (uint32_t len);

	Host *m_host;
	Udp *m_udp;
	Ipv4Address m_self_address;
	uint16_t m_self_port;
	Ipv4Address m_peer_address;
	uint16_t m_peer_port;
	uint32_t m_buffer_len;
	uint32_t m_used_buffer_len;
};

#endif /* SOCKET_UDP */
