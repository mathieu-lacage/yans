/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "function-holder.h"
#include "python-callback.tcc"
#include "method-callback.tcc"
#include "yans/tcp-sink.h"
#include "yans/host.h"
#include "yans/trace-container.h"

using namespace boost::python;
using namespace yans;

static void set_receive_callback_cpp (TcpSink *self, CppCallbackFactoryBase *factory)
{
	self->set_receive_callback (make_method_callback<void,Packet *> (factory));
}

static void set_receive_callback_python (TcpSink *self, FunctionHolder holder)
{
	self->set_receive_callback (make_python_callback<void, Packet *> (holder));
}

void export_tcp_sink (void)
{
	class_<TcpSink, boost::noncopyable> sink ("TcpSink", init<Host *> ());
	sink.def ("bind", &TcpSink::bind);
	sink.def ("stop_listen_at", &TcpSink::stop_listen_at);
	sink.def ("register_trace", &TcpSink::register_trace);

	sink.def ("set_receive_callback", &set_receive_callback_python);
	sink.def ("set_receive_callback", &set_receive_callback_cpp);
}
