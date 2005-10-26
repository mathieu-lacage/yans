/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef UDP_SINK_H
#define UDP_SINK_H

#include <stdint.h>
#include "ipv4-address.h"

class Host;
class Packet;
class Ipv4EndPoint;
class UdpSinkListener;

class UdpSink {
public:
	UdpSink (Host *host);
	~UdpSink ();

	bool bind (Ipv4Address address, uint16_t port);
private:
	friend class UdpSinkListener;
	void receive (Packet *packet);
	Host *m_host;
	Ipv4EndPoint *m_end_point;
	UdpSinkListener *m_listener;
};


#endif /* UDP_SINK_H */
