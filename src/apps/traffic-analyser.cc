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


#include "traffic-analyser.h"
#include "population-analysis.h"
#include "packet.h"
#include "simulator.h"
#include <iostream>

namespace yans {

TrafficAnalyser::TrafficAnalyser ()
{
	m_previous_arrival = 0;
	m_data = new PopulationAnalysis ();
	m_inter_arrival_time = new PopulationAnalysis ();
}
TrafficAnalyser::~TrafficAnalyser ()
{
	delete m_data;
	delete m_inter_arrival_time;
}


void 
TrafficAnalyser::receive (Packet *packet)
{
	m_data->add_term (packet->get_size ());
	uint64_t now_us = Simulator::now_us ();
	if (m_previous_arrival >= 0) {
		m_inter_arrival_time->add_term (now_us - m_previous_arrival);
	}
	m_previous_arrival = now_us;
}

void 
TrafficAnalyser::print_stats (void)
{
	double iat_mean_s = m_inter_arrival_time->get_mean () / 1000000;
	double iat_dev_s = m_inter_arrival_time->get_standard_deviation () / 1000000;
	std::cout << "received " << m_data->get_n () << " packets." << std::endl
		  << " packet size avg: " << m_data->get_mean () 
		  << ", std dev: " << m_data->get_standard_deviation () << std::endl
		  << " packet inter arrival avg: " << iat_mean_s
		  << ", std dev: " << iat_dev_s << std::endl
		  << "received " << m_data->get_total () << " bytes." << std::endl;
}

}; // namespace yans
