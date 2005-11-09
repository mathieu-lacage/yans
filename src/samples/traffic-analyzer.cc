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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */


#include "traffic-analyzer.h"


TrafficAnalyzer::TrafficAnalyzer ()
	: m_ref (this)
{}
TrafficAnalyzer::~TrafficAnalyzer ()
{}

void
TrafficAnalyzer::ref (void)
{
	m_ref.ref ();
}
void
TrafficAnalyzer::unref (void)
{
	m_ref.unref ();
}

void 
TrafficAnalyzer::receive (Packet *packet)
{}

uint32_t 
TrafficAnalyzer::get_n_packets_received (void)
{
	return 0;
}
uint32_t 
TrafficAnalyzer::get_n_bytes_received (void)
{
	return 0;
}
uint32_t 
TrafficAnalyzer::get_packet_size_mean (void)
{
	return 0;
}
uint32_t 
TrafficAnalyzer::get_packet_size_variance (void)
{
	return 0;
}
uint32_t 
TrafficAnalyzer::get_packet_interval_mean (void)
{
	return 0;
}
uint32_t 
TrafficAnalyzer::get_packet_interval_variance (void)
{
	return 0;
}

void 
TrafficAnalyzer::print_stats (void)
{}
