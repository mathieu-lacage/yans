/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

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
