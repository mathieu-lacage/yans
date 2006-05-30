/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <stdint.h>
#include <cassert>
#include <iostream>

#include "yans/simulator.h"
#include "yans/packet.h"
#include "yans/chunk-constant-data.h"
#include "yans/event.tcc"

using namespace yans;

namespace test {

class TestEventSource;

class TestConnector {
public:
	TestConnector (uint64_t delay_us);
	void connect (TestEventSource *source);
	void send (Packet *packet, TestEventSource *sender);
private:
	void forward_up (Packet *packet, TestEventSource *receiver);
	TestEventSource *m_a;
	TestEventSource *m_b;
	uint64_t m_delay_us;
};

class TestEventSource {
public:
	TestEventSource ();

	void start (uint64_t delta, uint64_t end);
	void connect (TestConnector *connector);
	void receive (Packet *packet);
	uint32_t get_received (void);
private:
	void send_one (void);
	uint64_t m_delta_us;
	uint64_t m_end_us;
	TestConnector *m_connector;
	uint32_t m_received;
};


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

TestEventSource::TestEventSource ()
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

};

using namespace test;

int main (int argc, char *argv) 
{
	TestConnector *c;
	TestEventSource *a, *b;
	a = new TestEventSource ();
	b = new TestEventSource ();
	c = new TestConnector (20);
	a->connect (c);
	b->connect (c);

	a->start (5, 10000);
	b->start (11, 10000);

	Simulator::run ();

	std::cout << "a received " <<a->get_received () << std::endl;
	std::cout << "b received " <<b->get_received () << std::endl;

	delete a;
	delete b;
	delete c;
	
	return 0;
}
