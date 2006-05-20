/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/ipv4-network-interface.h"

using namespace boost::python;
using namespace yans;

void export_ipv4_network_interface (void)
{
	class_<Ipv4NetworkInterface, boost::noncopyable> host ("Ipv4NetworkInterface", no_init);
}
