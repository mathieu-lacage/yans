/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "method-callback.tcc"
#include "yans/udp-source.h"
#include "yans/host.h"

using namespace boost::python;
using namespace yans;

void export_udp_source (void)
{
	class_<UdpSource, boost::noncopyable> udp ("UdpSource", init<Host *> ());
	udp.def ("bind", &UdpSource::bind);
	udp.def ("unbind_at", &UdpSource::unbind_at);
	udp.def ("set_peer", &UdpSource::set_peer);
	udp.def ("send", &UdpSource::send);

	EXPORT_METHOD_AS_CALLBACK (UdpSource,send);
}
