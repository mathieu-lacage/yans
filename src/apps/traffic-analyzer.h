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


#ifndef TRAFFIC_ANALYZER_H
#define TRAFFIC_ANALYZER_H

#include <stdint.h>

namespace yans {

class Packet;
class PopulationAnalysis;

class TrafficAnalyzer {
public:
	TrafficAnalyzer ();
	~TrafficAnalyzer ();

	void receive (Packet *packet);

	void print_stats (void);

private:
	PopulationAnalysis *m_data;
	PopulationAnalysis *m_inter_arrival_time;
	double m_previous_arrival;
};

}; // namespace yans



#endif /* TRAFFIC_ANALYZER_H */