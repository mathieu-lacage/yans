/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "throughput-printer.h"
#include "packet.h"
#include "simulator.h"
#include "event.tcc"
#include <iostream>

namespace yans {

ThroughputPrinter::ThroughputPrinter ()
	: m_current (0),
	  m_prev (0),
	  m_period_us (1000000)
{
	Simulator::insert_in_us (m_period_us, make_event (&ThroughputPrinter::timeout, this));
}
void 
ThroughputPrinter::set_print_interval_us (uint64_t us)
{
	m_period_us = us;
}
void 
ThroughputPrinter::receive (Packet const*packet)
{
	std::cout << "rx" << std::endl;
	m_current += packet->get_size ();
}

void
ThroughputPrinter::timeout (void)
{
	uint32_t n_bytes = m_current - m_prev;
	m_prev = m_current;
	double mbs = ((n_bytes * 8.0) /m_period_us);
	std::cout << "throughput="<<mbs<<"Mb/s"<<std::endl;
	Simulator::insert_in_us (m_period_us, make_event (&ThroughputPrinter::timeout, this));
}


}; // namespace yans
