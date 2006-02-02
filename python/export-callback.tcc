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

template<>
class ExportCallback0<void> : public Callback<void (void)> {
public:
	ExportCallback0 (FunctionHolder holder) 
		: m_holder (holder)
	{}
	virtual void operator() (void) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		function (context);
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
		return function (context, 
				 ExportCallbackTraits<T1>::make_arg (a1));
	}
private:
	FunctionHolder m_holder;
};

template<typename T1>
class ExportCallback1<void, T1> : public Callback<void (T1)> {
public:
	ExportCallback1 (FunctionHolder holder) 
		: m_holder (holder)
	{}
	virtual void operator() (T1 a1) {
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

template<typename T1, typename T2>
class ExportCallback2<void, T1, T2> : public Callback<void (T1, T2)> {
public:
	ExportCallback2 (FunctionHolder holder) 
		: m_holder (holder)
	{}
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


/* 
   magic to export various classes as callbacks.
*/

template<typename TAG, typename R, typename T, typename T1 = empty, typename T2 = empty>
class CppExportCallback : public CallbackBase {};
template<typename TAG, typename R, typename T, typename T1>
class CppExportCallback<TAG,R,T,T1> : public Callback<R (T1)> {
public:
	typedef R (T::*F) (T1);

	CppExportCallback (T *obj) 
		: m_obj (obj) {}
	virtual R operator() (T1 a1) {
		return (m_obj->*m_member) (a1);
	}
	static F m_member;
private:
	CppExportCallback () {}
	T *m_obj;
};

template<typename TAG,typename R,typename T,typename T1>
R (T::* CppExportCallback<TAG,R,T,T1>::m_member) (T1);

template <typename TAG, typename R, typename T, typename T1>
void
export_method_as_callback (char const *name, R (T::*member) (T1))
{
	CppExportCallback<TAG,R,T,T1>::m_member = member;
	class_<CppExportCallback<TAG,R,T,T1>, 
		bases<CallbackBase>,
		std::auto_ptr<CppExportCallback<TAG,R,T,T1> >,
		boost::noncopyable> 
		(name, init<T *> ());

	implicitly_convertible<std::auto_ptr<CppExportCallback<TAG,R,T,T1> >,
		std::auto_ptr<CallbackBase> >();
}


#endif /* EXPORT_CALLBACK_TCC */
