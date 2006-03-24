/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "export-callback.tcc"
#include "yans/tcp-source.h"
#include "yans/host.h"
#include "yans/trace-container.h"

using namespace boost::python;
using namespace yans;

void export_tcp_source (void)
{
	class_<TcpSource, boost::noncopyable> tcp ("TcpSource", init<Host *> ());
	tcp.def ("bind", &TcpSource::bind);
	tcp.def ("send", &TcpSource::send);
	tcp.def ("start_connect_at", &TcpSource::start_connect_at);
	tcp.def ("start_connect_now", &TcpSource::start_connect_now);
	tcp.def ("start_disconnect_now", &TcpSource::start_disconnect_at);
	tcp.def ("register_trace", &TcpSource::register_trace);

	export_method_as_callback<struct TcpSource_send> ("TcpSource_send_callback", &TcpSource::send);
}
