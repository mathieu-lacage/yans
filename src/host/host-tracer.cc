/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "host-tracer.h"
#include "packet.h"
#include "simulator.h"
#include <iostream>

void 
HostTracer::trace_tx_app (Packet *packet)
{
	if (m_enable_all || m_enable_app) {
		std::cout << Simulator::instance ()->now_s ()
			  << " tx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
HostTracer::trace_tx_udp (Packet *packet)
{
	if (m_enable_all || m_enable_udp) {
		std::cout << Simulator::instance ()->now_s ()
			  << " tx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
HostTracer::trace_tx_ipv4 (Packet *packet)
{
	if (m_enable_all || m_enable_ipv4) {
		std::cout << Simulator::instance ()->now_s ()
			  << " tx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
HostTracer::trace_rx_app (Packet *packet)
{
	if (m_enable_all || m_enable_app) {
		std::cout << Simulator::instance ()->now_s ()
			  << " rx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
HostTracer::trace_rx_udp (Packet *packet)
{
	if (m_enable_all || m_enable_udp) {
		std::cout << Simulator::instance ()->now_s ()
			  << " rx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
HostTracer::trace_rx_ipv4 (Packet *packet)
{
	if (m_enable_all || m_enable_ipv4) {
		std::cout << Simulator::instance ()->now_s ()
			  << " rx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}

void 
HostTracer::enable_all (void)
{
	m_enable_all = true;
}
void 
HostTracer::disable_all (void)
{
	m_enable_all = false;
}
HostTracer::HostTracer (WriteFile *file)
	: m_enable_all (),
	  m_enable_app (false),
	  m_enable_udp (false),
	  m_enable_ipv4 (false),
	  m_file (file)
{}
HostTracer::~HostTracer ()
{}