/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "tcp-sink.h"
#include "function-holder.h"
#include "export-callback.tcc"
#include "host.h"
#include "trace-container.h"

using namespace boost::python;
using namespace yans;

static void set_receive_callback_cpp (TcpSink *self, std::auto_ptr<CallbackBase> callback)
{
	self->set_receive_callback (static_cast<Callback<void (Packet *)> *> (callback.get ()));
	callback.release ();
}
static void set_receive_callback_python (TcpSink *self, FunctionHolder holder)
{
	self->set_receive_callback (new ExportCallback1<void, Packet *> (holder));
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
