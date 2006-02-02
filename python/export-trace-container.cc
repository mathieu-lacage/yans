/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "trace-container.h"
#include "export-callback.tcc"
#include "callback.tcc"
#include "packet.h"

using namespace boost::python;
using namespace yans;

static void set_ui_variable_callback_cpp (TraceContainer *self, char const *name, std::auto_ptr<CallbackBase> callback)
{
	self->set_ui_variable_callback (name, static_cast <Callback<void (uint64_t, uint64_t)> *> (callback.get ()));
	callback.release ();
}

static void set_ui_variable_callback_python (TraceContainer *self, char const *name, FunctionHolder holder)
{
	self->set_ui_variable_callback (name, new ExportCallback2<void, uint64_t, uint64_t> (holder));
}

static void set_si_variable_callback_cpp (TraceContainer *self, char const *name, std::auto_ptr<CallbackBase> callback)
{
	self->set_si_variable_callback (name, static_cast <Callback<void (int64_t, int64_t)> *> (callback.get ()));
	callback.release ();
}

static void set_si_variable_callback_python (TraceContainer *self, char const *name, FunctionHolder holder)
{
	self->set_si_variable_callback (name, new ExportCallback2<void, int64_t, int64_t> (holder));
}

static void set_packet_logger_callback_cpp (TraceContainer *self, char const *name, std::auto_ptr<CallbackBase> callback)
{
	self->set_packet_logger_callback (name, static_cast <Callback<void (Packet *)> *> (callback.get ()));
	callback.release ();
}

static void set_packet_logger_callback_python (TraceContainer *self, char const *name, FunctionHolder holder)
{
	self->set_packet_logger_callback (name, new ExportCallback1<void, Packet *> (holder));
}

void export_trace_container (void)
{
	class_<TraceContainer, boost::noncopyable> container ("TraceContainer");
	container.def ("set_ui_variable_callback", &set_ui_variable_callback_cpp);
	container.def ("set_ui_variable_callback", &set_ui_variable_callback_python);
	container.def ("set_si_variable_callback", &set_si_variable_callback_cpp);
	container.def ("set_si_variable_callback", &set_si_variable_callback_python);
	container.def ("set_packet_logger_callback", &set_packet_logger_callback_cpp);
	container.def ("set_packet_logger_callback", &set_packet_logger_callback_python);
	container.def ("print_debug", &TraceContainer::print_debug);
#if 0
	void set_si_variable_callback (char const *name, Callback<void (int64_t, int64_t)> *callback);
	void set_f_variable_callback (char const *name, Callback<void (double, double)> *callback);
	void set_packet_logger_callback (char const *name, Callback<void (Packet *)> *callback);
#endif
}
