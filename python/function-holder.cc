/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "function-holder.h"

#include <boost/python.hpp>

using namespace boost::python;


FunctionHolder::FunctionHolder (object function, 
				object context)
	: m_function (function), m_context (context)
{}

object 
FunctionHolder::get_function (void)
{
	return m_function;
}
object 
FunctionHolder::get_context (void)
{
	return m_context;
}

void 
export_function_holder (void)
{
	class_<FunctionHolder, std::auto_ptr<FunctionHolder>, boost::noncopyable> ("FunctionHolder", init<object, object> ());
}
