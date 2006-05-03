/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "method-callback.tcc"
#include "yans/traffic-analyser.h"
#include "yans/callback.h"
#include "yans/packet.h"

using namespace boost::python;
using namespace yans;

void export_traffic_analyser (void)
{
	class_<TrafficAnalyser, boost::noncopyable> analyser ("TrafficAnalyser");
	analyser.def ("print_stats", &TrafficAnalyser::print_stats);
	analyser.def ("receive", &TrafficAnalyser::receive);

	EXPORT_METHOD_AS_CALLBACK (TrafficAnalyser,receive);
}
