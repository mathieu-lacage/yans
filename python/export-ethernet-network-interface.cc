/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "ethernet-network-interface.h"

using namespace boost::python;
using namespace yans;

void export_ethernet_network_interface (void)
{
	class_<EthernetNetworkInterface, 
		bases<NetworkInterface>, 
		std::auto_ptr<EthernetNetworkInterface>,
		boost::noncopyable> 
		ethernet
		("EthernetNetworkInterface", 
		 init<char const *> ());
	implicitly_convertible<std::auto_ptr<EthernetNetworkInterface>,
		std::auto_ptr<NetworkInterface> >();

	ethernet.def ("set_mtu", &EthernetNetworkInterface::set_mtu);
}
