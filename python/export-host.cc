/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/host.h"
#include "yans/mac-network-interface.h"
#include "yans/ipv4-network-interface.h"
#include "yans/ipv4-route.h"

using namespace boost::python;
using namespace yans;

static Ipv4NetworkInterface *
add_ipv4_arp_interface (Host *host, MacNetworkInterface *interface, 
			Ipv4Address address, Ipv4Mask mask)
{
	return host->add_ipv4_arp_interface (interface, address, mask);
}


void export_host (void)
{
	class_<Host, boost::noncopyable> host ("Host", init<char const *> ());
	host.def ("add_ipv4_arp_interface", 
		  &add_ipv4_arp_interface, 
		  return_internal_reference<1, 
		  with_custodian_and_ward<1,2> >());
	host.def ("get_routing_table", &Host::get_routing_table, return_internal_reference<1> ());
}
