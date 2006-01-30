/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "ipv4-route.h"
#include "network-interface.h"

using namespace boost::python;
using namespace yans;

static void
set_default_route (Ipv4Route *route,
		   Ipv4Address next_hop, 
		   std::auto_ptr<NetworkInterface> interface)
{
	route->set_default_route (next_hop, interface.get ());
}

void export_ipv4_route (void)
{
	class_<Ipv4Route, boost::noncopyable> table ("Ipv4RouteTable", no_init);
	table.def ("set_default_route", &set_default_route);
}
