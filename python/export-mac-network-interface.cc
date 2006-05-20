/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/mac-network-interface.h"

using namespace boost::python;
using namespace yans;

void export_mac_network_interface (void)
{
	class_<MacNetworkInterface, 
		boost::noncopyable> 
		interface 
		("MacNetworkInterface", no_init);
	interface.def ("set_mtu", &MacNetworkInterface::set_mtu);
	interface.def ("get_mtu", &MacNetworkInterface::get_mtu);
}
