/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "ethernet-network-interface.h"

using namespace boost::python;
using namespace yans;

void export_ethernet_network_interface (void)
{
	class_<EthernetNetworkInterface, 
		bases<NetworkInterface>, 
		boost::noncopyable> 
		ethernet
		("EthernetNetworkInterface", 
		 init<char const *> ());
	ethernet.def ("set_mtu", &EthernetNetworkInterface::set_mtu);
}
