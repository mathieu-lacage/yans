/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <stdint.h>
#include <cassert>

#include "yans/simulator.h"
#include "yans/packet.h"
#include "yans/chunk-constant-data.h"
#include "yans/event.tcc"

#include "local.h"

using namespace yans;

namespace local {

ComputingContext::ComputingContext ()
{}

ComputingContext *
ComputingContextFactory::create (char const *name)
{
	return new ComputingContext ();
}

TestConnector::TestConnector (uint64_t delay_us)
	: m_a (0), m_b(0), m_delay_us (delay_us)
{}

void
TestConnector::connect (TestEventSource *source)
{
	assert (m_a == 0 || m_b == 0);
	if (m_a == 0) {
		m_a = source;
	} else {
		m_b = source;
	}
}
void 
TestConnector::send (Packet *packet, TestEventSource *sender)
{
	TestEventSource *receiver;
	if (m_a == sender) {
		receiver = m_b;
	} else {
		receiver = m_a;
	}
	packet->ref ();
	Simulator::insert_in_us (m_delay_us,
				 make_event (&TestConnector::forward_up, this, packet, receiver));
}
void
TestConnector::forward_up (Packet *packet, TestEventSource *receiver)
{
	receiver->receive (packet);
	packet->unref ();
}

TestEventSource::TestEventSource (ComputingContext *ctx)
	: m_received (0)
{}

void 
TestEventSource::connect (TestConnector *connector)
{
	connector->connect (this);
	m_connector = connector;
}

void
TestEventSource::send_one (void)
{
	if (Simulator::now_us () > m_end_us) {
		return;
	}
	Packet *p = PacketFactory::create ();
	ChunkConstantData data = ChunkConstantData (1000, 2);
	p->add (&data);
	m_connector->send (p, this);
	p->unref ();
	Simulator::insert_in_us (m_delta_us,
				 make_event (&TestEventSource::send_one, this));
}

void
TestEventSource::start (uint64_t delta, uint64_t end)
{
	m_delta_us = delta;
	m_end_us = end;
	Simulator::insert_in_us (delta, 
				 make_event (&TestEventSource::send_one, this));
}

void 
TestEventSource::receive (Packet *packet)
{
	m_received++;
}

uint32_t 
TestEventSource::get_received (void)
{
	return m_received;
}

}; // namespace local
