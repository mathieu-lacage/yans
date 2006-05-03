/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "function-holder.h"
#include "python-callback.tcc"
#include "method-callback.tcc"

#include "yans/udp-sink.h"
#include "yans/host.h"

using namespace boost::python;
using namespace yans;

static void set_receive_callback_cpp (UdpSink *self, CppCallbackFactoryBase *factory)
{
	self->set_receive_callback (make_method_callback<void,Packet *> (factory));
}

static void set_receive_callback_python (UdpSink *self, FunctionHolder holder)
{
	self->set_receive_callback (make_python_callback<void, Packet *> (holder));
}



void export_udp_sink (void)
{
	class_<UdpSink, boost::noncopyable> sink ("UdpSink", init<Host *> ());
	sink.def ("bind", &UdpSink::bind);
	sink.def ("unbind_at", &UdpSink::unbind_at);
	sink.def ("set_receive_callback", &set_receive_callback_python);
	sink.def ("set_receive_callback", &set_receive_callback_cpp);
}
