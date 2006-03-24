/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/cable.h"
#include "yans/ethernet-network-interface.h"

using namespace boost::python;
using namespace yans;

void export_cable (void)
{
	class_<Cable, 
		boost::noncopyable> 
		cable
		("Cable", init<double, double> ());
	cable.def (init<> ());
	cable.def ("connect_to", &Cable::connect_to);
}
