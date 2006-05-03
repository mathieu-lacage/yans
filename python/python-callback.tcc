/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef PYTHON_CALLBACK_TCC
#define PYTHON_CALLBACK_TCC

#include "boost/python.hpp"
#include "yans/callback.h"
#include "function-holder.h"
#include "export-callback-traits.tcc"

using namespace yans;
using namespace boost::python;

template<typename R,typename T1,typename T2>
class PythonCallbackImpl : public CallbackImpl<R,T1,T2,empty,empty> {
public:
	PythonCallbackImpl (FunctionHolder holder) 
		: m_holder (holder)
	{}
	~PythonCallbackImpl () {}
	virtual R operator() (void) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		return function (context);
	}
	virtual R operator() (T1 a1) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		return function (context,
				 ExportCallbackTraits<T1>::make_arg (a1));
	}
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

template<typename T1,typename T2>
class PythonCallbackImpl<void,T1,T2> : public CallbackImpl<void,T1,T2,empty,empty> {
public:
	PythonCallbackImpl (FunctionHolder holder) 
		: m_holder (holder)
	{}
	~PythonCallbackImpl () {}
	virtual void operator() (void) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		function (context);
	}
	virtual void operator() (T1 a1) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		function (context,
			  ExportCallbackTraits<T1>::make_arg (a1));
	}
	virtual void operator() (T1 a1, T2 a2) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		function (context,
			  ExportCallbackTraits<T1>::make_arg (a1),
			  ExportCallbackTraits<T2>::make_arg (a2));
	}
private:
	FunctionHolder m_holder;
};


template <typename R,typename T1>
Callback<R,T1> make_python_callback (FunctionHolder holder) {
	return Callback<R,T1> (ReferenceList<CallbackImpl<R,T1,empty,empty,empty>*> (new PythonCallbackImpl<R,T1,empty> (holder)));
}
template <typename R,typename T1,typename T2>
Callback<R,T1,T2> make_python_callback (FunctionHolder holder) {
	return Callback<R,T1,T2> (ReferenceList<CallbackImpl<R,T1,T2,empty,empty>*> (new PythonCallbackImpl<R,T1,T2> (holder)));
}



#endif /* PYTHON_CALLBACK_TCC */
