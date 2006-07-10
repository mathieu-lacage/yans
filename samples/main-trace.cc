/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#include "yans/trace-container.h"
#include "yans/ui-traced-variable.tcc"
#include "yans/packet-logger.h"
#include "yans/trace-stream.h"
#include "yans/pcap-writer.h"
#include <iostream>

using namespace yans;

PacketLogger a;
UiTracedVariable<unsigned short> b;
TraceStream c;

void
register_all_trace_sources (TraceContainer *container)
{
	container->register_packet_logger ("source-a", &a);
	container->register_ui_variable ("source-b", &b);
	container->register_stream ("source-c", &c);
}
void
generate_trace_events (void)
{
	// log en empty packet
	a.log (Packet::create ());
	b = 10;
	b += 100;
	b += 50;
	b = (unsigned short) -20;
	c << "this is a simple test b=" << b << std::endl;
}

void
variable_event (uint64_t old, uint64_t cur)
{}


int main (int argc, char *argv[])
{
	TraceContainer traces;
	register_all_trace_sources (&traces);
	PcapWriter pcap;
	pcap.open ("trace-test.log");
	pcap.write_header_ethernet ();
	traces.set_packet_logger_callback ("source-a", 
					   make_callback (&PcapWriter::write_packet, &pcap));
	traces.set_ui_variable_callback ("source-b", make_callback (&variable_event));
	traces.set_stream ("source-c", &std::cout);
	generate_trace_events ();
	return 0;
}
