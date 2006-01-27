/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "callback.tcc"

using namespace boost::python;
using namespace yans;

void export_callback (void)
{
	class_<CallbackBase, std::auto_ptr<CallbackBase>, boost::noncopyable> ("CallbackBase", no_init);
}
