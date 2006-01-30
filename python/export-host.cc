/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "host.h"
#include "network-interface.h"
#include "ipv4-route.h"

using namespace boost::python;
using namespace yans;

static void 
add_interface (Host *host, std::auto_ptr<NetworkInterface> interface)
{
	host->add_interface (interface.get ());
	interface.release ();
}


void export_host (void)
{
	class_<Host, boost::noncopyable> host ("Host", init<char const *> ());
	host.def ("add_interface", &add_interface);
	host.def ("get_routing_table", &Host::get_routing_table);
}
