/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "udp-source.h"
#include "event.h"
#include "packet.h"
#include "chunk-fake-data.h"
#include "ipv4-endpoint.h"
#include "host.h"
#include "udp.h"
#include "simulator.h"
#include "tag-ipv4.h"
#include "tracer.h"

class UdpSourceEvent : public Event {
public:
	UdpSourceEvent (UdpSource *source);
	virtual ~UdpSourceEvent ();

	virtual void notify (void);
	virtual void notify_canceled (void);
private:
	UdpSource *m_source;
};

UdpSourceEvent::UdpSourceEvent (UdpSource *source)
	: m_source (source)
{}
UdpSourceEvent::~UdpSourceEvent ()
{}
void 
UdpSourceEvent::notify (void)
{
	m_source->send_next_packet ();
}
void 
UdpSourceEvent::notify_canceled (void)
{}

class UdpSourceListener : public Ipv4EndPointListener {
public:
	UdpSourceListener ();
	virtual ~UdpSourceListener ();
	virtual void receive (Packet *packet);
};
UdpSourceListener::UdpSourceListener ()
{}
UdpSourceListener::~UdpSourceListener ()
{}
void UdpSourceListener::receive (Packet *packet)
{
	packet->unref ();
}




UdpSource::UdpSource (Host *host)
	: m_host (host),
	  m_end_point (0)
{}
UdpSource::~UdpSource ()
{
	if (m_end_point != 0) {
		Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
		end_points->destroy (m_end_point);
	}
	m_end_point = (Ipv4EndPoint *) 0xdeadbeaf;
	m_host = (Host *)0xdeadbeaf;
}
bool 
UdpSource::bind (Ipv4Address address, uint16_t port)
{
	Ipv4EndPoints *end_points = m_host->get_udp ()->get_end_points ();
	m_end_point = end_points->allocate (new UdpSourceListener (), address, port);
	if (m_end_point == 0) {
		return false;
	}
	return true;
}
void 
UdpSource::set_peer (Ipv4Address address, uint16_t port)
{
	m_peer_address = address;
	m_peer_port = port;
}
void 
UdpSource::set_packet_interval (double interval)
{
	m_interval = interval;
}
void 
UdpSource::set_packet_size (uint16_t size)
{
	m_size = size;
}

void 
UdpSource::start_at (double start)
{
	Simulator::instance ()->insert_at_s (new UdpSourceEvent (this), start);
}
void 
UdpSource::stop_at (double end)
{
	m_stop_at = end;
}


void 
UdpSource::send_next_packet (void)
{
	/* stop packet transmissions.*/
	if (m_stop_at > 0.0 && Simulator::instance ()->now_s () >= m_stop_at) {
		return;
	}
	/* schedule next packet transmission. */
	Simulator::instance ()->insert_in_s (new UdpSourceEvent (this), m_interval);
	/* create packet. */
	Packet *packet = new Packet ();
	ChunkFakeData *data = new ChunkFakeData (m_size);
	packet->add_header (data);
	/* route packet. */
	Ipv4Route *routing_table = m_host->get_routing_table ();
	Route *route = routing_table->lookup (m_peer_address);
	TagOutIpv4 *tag = new TagOutIpv4 (route);
	tag->set_sport (m_end_point->get_port ());
	tag->set_dport (m_peer_port);
	tag->set_daddress (m_peer_address);
	tag->set_saddress (m_end_point->get_address ());
	packet->add_tag (TagOutIpv4::get_tag (), tag);
	/* send packet. */
	m_host->get_tracer ()->trace_tx_app (packet);
	m_host->get_udp ()->send (packet);
}
