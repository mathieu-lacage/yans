/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "host.h"
#include "network-interface.h"
#include "ipv4-route.h"

using namespace boost::python;
using namespace yans;

static void 
add_interface (Host *host, NetworkInterface *interface)
{
	host->add_interface (interface);
}


void export_host (void)
{
	class_<Host, boost::noncopyable> host ("Host", init<char const *> ());
	host.def ("add_interface", &add_interface, with_custodian_and_ward<1,2> ());
	host.def ("get_routing_table", &Host::get_routing_table, return_internal_reference<1> ());
}
