/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef LOCAL_H
#define LOCAL_H

#include <stdint.h>

namespace yans {
  class Packet;
};

namespace local {

class ComputingContext {
public:
private:
	ComputingContext ();
	friend class ComputingContextFactory;
};
class ComputingContextFactory {
public:
	static ComputingContext *create (char const *name);
private:
	ComputingContextFactory ();
};
class TestEventSource;

class TestConnector {
public:
	TestConnector (uint64_t delay_us);
	void connect (TestEventSource *source);
	void send (yans::Packet *packet, TestEventSource *sender);
private:
	void forward_up (yans::Packet *packet, TestEventSource *receiver);
	TestEventSource *m_a;
	TestEventSource *m_b;
	uint64_t m_delay_us;
};

class TestEventSource {
public:
	TestEventSource (ComputingContext *ctx);

	void start (uint64_t delta, uint64_t end);
	void connect (TestConnector *connector);
	void receive (yans::Packet *packet);
	uint32_t get_received (void);
private:
	void send_one (void);
	uint64_t m_delta_us;
	uint64_t m_end_us;
	TestConnector *m_connector;
	uint32_t m_received;
};



}; // namespace test

#endif /* LOCAL_H */
