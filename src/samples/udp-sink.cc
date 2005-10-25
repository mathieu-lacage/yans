/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "udp-sink.h"
#include "udp.h"
#include "host.h"
#include "ipv4-endpoint.h"
#include "packet.h"
#include "tracer.h"


class UdpSinkListener : public Ipv4EndPointListener {
public:
	UdpSinkListener (UdpSink *sink);
	virtual ~UdpSinkListener ();
	virtual void receive (Packet *packet);
private:
	UdpSink *m_sink;
};
UdpSinkListener::UdpSinkListener (UdpSink *sink)
	: m_sink (sink)
{}
UdpSinkListener::~UdpSinkListener ()
{}
void UdpSinkListener::receive (Packet *packet)
{
	m_sink->receive (packet);
}


UdpSink::UdpSink (Host *host)
	: m_host (host),
	  m_end_point (0)
{}

bool 
UdpSink::bind (Ipv4Address address, uint16_t port)
{
	Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
	m_end_point = end_points->allocate (new UdpSinkListener (this), address, port);
	if (m_end_point == 0) {
		return false;
	}
	return true;
}

void
UdpSink::receive (Packet *packet)
{
	m_host->get_tracer ()->trace_rx_app (packet);
	packet->unref ();
}
