/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "network-interface.h"
#include "network-interface-tracer.h"

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
	interface.def ("get_tracer", &NetworkInterface::get_tracer, return_internal_reference<1> ());

	class_<NetworkInterfaceTracer, boost::noncopyable> tracer 
		("NetworkInterfaceTracer", no_init);
	tracer.def ("enable_all", &NetworkInterfaceTracer::enable_all);
							    
}
