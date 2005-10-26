/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef UDP_SOURCE_H
#define UDP_SOURCE_H

#include <stdint.h>
#include "ipv4-address.h"
#include "ipv4-endpoint.h"

class Host;
class Ipv4EndPoint;
class UdpSourceListener;

class UdpSource {
public:
	UdpSource (Host *host);
	~UdpSource ();

	/* return true on success. */
	bool bind (Ipv4Address address, uint16_t port);

	void set_peer (Ipv4Address address, uint16_t port);

	void set_packet_interval (double interval);
	void set_packet_size (uint16_t size);

	void start_at (double start);
	void stop_at (double end);

private:
	friend class UdpSourceEvent;
	void send_next_packet (void);

	Host *m_host;
	double m_interval;
	uint16_t m_size;
	double m_stop_at;
	Ipv4EndPoint *m_end_point;
	Ipv4Address m_peer_address;
	uint16_t m_peer_port;
	UdpSourceListener *m_listener;
};

#endif /* UDP_SOURCE_H */
