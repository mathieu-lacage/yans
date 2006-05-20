/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/ethernet-network-interface.h"
#include "yans/trace-container.h"

using namespace boost::python;
using namespace yans;

void export_ethernet_network_interface (void)
{
	class_<EthernetNetworkInterface, 
		bases<MacNetworkInterface>, 
		std::auto_ptr<EthernetNetworkInterface>,
		boost::noncopyable> 
		ethernet
		("EthernetNetworkInterface", 
		 init<MacAddress, char const *> ());
	implicitly_convertible<std::auto_ptr<EthernetNetworkInterface>,
		std::auto_ptr<MacNetworkInterface> >();

	ethernet.def ("register_trace", &EthernetNetworkInterface::register_trace);
}
