/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "host.h"
#include "network-interface.h"

using namespace boost::python;
using namespace yans;

void export_host (void)
{
	class_<Host, boost::noncopyable> host ("Host", init<char const *> ());
	host.def ("add_interface", &Host::add_interface);
}
