/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "mac-address.h"

using namespace boost::python;
using namespace yans;

void export_mac_address (void)
{
	class_<MacAddress, boost::noncopyable> ("MacAddress", init<char const *> ());
}
