/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/ipv4-address.h"

using namespace boost::python;
using namespace yans;

void export_ipv4_address (void)
{
	class_<Ipv4Address, boost::noncopyable> ipv4 ("Ipv4Address", init<char const *> ());
	class_<Ipv4Mask, boost::noncopyable> mask ("Ipv4Mask", init<char const *> ());
}
