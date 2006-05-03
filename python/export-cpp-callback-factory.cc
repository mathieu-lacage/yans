/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "method-callback.tcc"

using namespace boost::python;

void
export_cpp_callback_factory (void)
{
  class_<CppCallbackFactoryBase, boost::noncopyable> ("CppCallbackFactoryBase", no_init);
}
