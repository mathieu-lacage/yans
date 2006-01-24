/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FUNCTION_HOLDER_H
#define FUNCTION_HOLDER_H

#include <boost/python.hpp>

class FunctionHolder {
public:
	FunctionHolder (boost::python::object function, boost::python::object context);

	boost::python::object get_function (void);
	boost::python::object get_context (void);
private:
	boost::python::object m_function;
	boost::python::object m_context;
};

#endif /* FUNCTION_HOLDER_H */
