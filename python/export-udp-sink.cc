/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "udp-sink.h"
#include "host.h"

using namespace boost::python;
using namespace yans;

void export_udp_sink (void)
{
	class_<UdpSink, boost::noncopyable> ("UdpSink", init<Host *> ());
}
