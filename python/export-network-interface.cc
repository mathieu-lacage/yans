/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "network-interface.h"

using namespace boost::python;
using namespace yans;

void export_network_interface (void)
{
	class_<NetworkInterface, 
		boost::noncopyable> 
		interface 
		("NetworkInterface", no_init);
	interface.def ("set_mac_address", &NetworkInterface::set_mac_address);
	interface.def ("set_ipv4_address", &NetworkInterface::set_ipv4_address);
	interface.def ("set_ipv4_mask", &NetworkInterface::set_ipv4_mask);
	interface.def ("set_up", &NetworkInterface::set_up);							    
}
