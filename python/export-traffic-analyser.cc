/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "traffic-analyser.h"
#include "export-callback.tcc"
#include "packet.h"

using namespace boost::python;
using namespace yans;


void export_traffic_analyser (void)
{
	class_<TrafficAnalyser, boost::noncopyable> analyser ("TrafficAnalyser");
	analyser.def ("print_stats", &TrafficAnalyser::print_stats);
	analyser.def ("receive", &TrafficAnalyser::receive);
}
