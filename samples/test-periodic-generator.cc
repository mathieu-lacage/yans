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

#include "yans/simulator.h"
#include "yans/periodic-generator.h"
#include "yans/traffic-analyser.h"
#include "yans/callback.h"

using namespace yans;


int main (int argc, char *argv[])
{

	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.00001);
	generator->set_packet_size (100);
	generator->start_at (0.0);
	generator->stop_at (10.0);

	TrafficAnalyser *analyser = new TrafficAnalyser ();
	generator->set_send_callback (make_callback (&TrafficAnalyser::receive, analyser));

	/* run simulation */
	Simulator::run ();
	analyser->print_stats ();

	/* destroy network */
	delete generator;
	delete analyser;
	Simulator::destroy ();

	return 0;
}
