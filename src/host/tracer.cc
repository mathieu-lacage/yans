/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "tracer.h"
#include "packet.h"
#include <iostream>

void 
Tracer::trace_tx_app (Packet *packet)
{
	if (m_enable_all || m_enable_app) {
		std::cout << "tx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
Tracer::trace_tx_udp (Packet *packet)
{
	if (m_enable_all || m_enable_udp) {
		std::cout << "tx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
Tracer::trace_tx_ipv4 (Packet *packet)
{
	if (m_enable_all || m_enable_ipv4) {
		std::cout << "tx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
Tracer::trace_rx_app (Packet *packet)
{
	if (m_enable_all || m_enable_app) {
		std::cout << "rx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
Tracer::trace_rx_udp (Packet *packet)
{
	if (m_enable_all || m_enable_udp) {
		std::cout << "rx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}
void 
Tracer::trace_rx_ipv4 (Packet *packet)
{
	if (m_enable_all || m_enable_ipv4) {
		std::cout << "rx ";
		packet->print (&std::cout);
		std::cout << std::endl;
	}
}

void 
Tracer::enable_all (void)
{
	m_enable_all = true;
}
void 
Tracer::disable_all (void)
{
	m_enable_all = false;
}
Tracer::Tracer (WriteFile *file)
	: m_enable_all (),
	  m_enable_app (false),
	  m_enable_udp (false),
	  m_enable_ipv4 (false),
	  m_file (file)
{}
Tracer::~Tracer ()
{}
