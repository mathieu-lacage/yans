/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/ipv4-route.h"
#include "yans/ipv4-network-interface.h"

using namespace boost::python;
using namespace yans;

void export_ipv4_route (void)
{
	class_<Ipv4Route, boost::noncopyable> table ("Ipv4RouteTable", no_init);
	table.def ("set_default_route", &Ipv4Route::set_default_route, with_custodian_and_ward<1, 3> ());
}
