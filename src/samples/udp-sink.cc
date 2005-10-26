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
	  m_end_point (0),
	  m_listener (new UdpSinkListener (this))
{}

UdpSink::~UdpSink ()
{
	if (m_end_point != 0) {
		Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
		end_points->destroy (m_end_point);
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
	delete m_listener;
	m_listener = (UdpSinkListener *)0xdeadbeaf;
}

bool 
UdpSink::bind (Ipv4Address address, uint16_t port)
{
	assert (m_end_point == 0);
	Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
	m_end_point = end_points->allocate (m_listener, address, port);
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
