/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "periodic-generator.h"
#include "export-callback.tcc"
#include "callback.tcc"
#include "packet.h"

using namespace boost::python;
using namespace yans;

void set_send_callback_cpp (PeriodicGenerator *self, Callback<void (Packet *)> *callback)
{
	self->set_send_callback (callback);
}

void set_send_callback_python (PeriodicGenerator *self, FunctionHolder holder)
{
	self->set_send_callback (new ExportCallback1<void, Packet *> (holder));
}

void export_periodic_generator (void)
{
	class_<PeriodicGenerator, boost::noncopyable> generator ("PeriodicGenerator");
	generator.def ("set_packet_interval", &PeriodicGenerator::set_packet_interval);
	generator.def ("set_packet_size", &PeriodicGenerator::set_packet_size);
	generator.def ("start_at", &PeriodicGenerator::start_at);
	generator.def ("stop_at", &PeriodicGenerator::stop_at);

	generator.def ("set_send_callback", &set_send_callback_python);
	generator.def ("set_send_callback", &set_send_callback_cpp);
}
