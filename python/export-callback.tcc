/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef EXPORT_CALLBACK_TCC
#define EXPORT_CALLBACK_TCC

#include "callback.tcc"
#include "function-holder.h"
#include "boost/python.hpp"
#include "export-callback-traits.tcc"

using namespace yans;
using namespace boost::python;


template<typename R>
class ExportCallback0 : public Callback<R (void)> {
public:
	ExportCallback0 (FunctionHolder holder) 
		: m_holder (holder)
	{}
	virtual R operator() (void) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		return function (context);
	}
private:
	FunctionHolder m_holder;
};

template<typename R, typename T1>
class ExportCallback1 : public Callback<R (T1)> {
public:
	ExportCallback1 (FunctionHolder holder) 
		: m_holder (holder)
	{}
	virtual R operator() (T1 a1) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		function (context, 
			  ExportCallbackTraits<T1>::make_arg (a1));
	}
private:
	FunctionHolder m_holder;
};

template<typename R, typename T1, typename T2>
class ExportCallback2 : public Callback<R (T1, T2)> {
public:
	ExportCallback2 (FunctionHolder holder) 
		: m_holder (holder)
	{}
	virtual R operator() (T1 a1, T2 a2) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		return function (context, 
				 ExportCallbackTraits<T1>::make_arg (a1), 
				 ExportCallbackTraits<T2>::make_arg (a2));
	}
private:
	FunctionHolder m_holder;
};

#endif /* EXPORT_CALLBACK_TCC */
