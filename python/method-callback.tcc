/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef METHOD_CALLBACK_TCC
#define METHOD_CALLBACK_TCC

#include "boost/python.hpp"
#include "yans/callback.h"
#include "export-callback-traits.tcc"

using namespace yans;
using namespace boost::python;

// declare a base class for the factory with a virtual destructor
class CppCallbackFactoryBase {
public:
	virtual ~CppCallbackFactoryBase () {}
};

// declare the factories.
template <typename R, typename T1>
class CppCallbackFactory1 : public CppCallbackFactoryBase {
public:
	~CppCallbackFactory1 () {}
	virtual Callback<R,T1> make_callback (void) const = 0;
};
template <typename R, typename T1,typename T2>
class CppCallbackFactory2 : public CppCallbackFactoryBase {
public:
	~CppCallbackFactory2 () {}
	virtual Callback<R,T1,T2> make_callback (void) const = 0;
};


// implement a factory for a 1-arg member method.
template <typename TAG, typename OBJ_PTR, typename MEM_PTR, typename R, typename T1>
class MethodPtrCppCallbackFactory1 : public CppCallbackFactory1<R,T1> {
public:
	MethodPtrCppCallbackFactory1 (OBJ_PTR obj_ptr)
		: m_obj_ptr (obj_ptr) {}
	~MethodPtrCppCallbackFactory1 () {}
	Callback<R,T1> make_callback (void) const {
		return yans::make_callback (m_mem_ptr, m_obj_ptr);
	}
	static MEM_PTR m_mem_ptr;
private:
	OBJ_PTR m_obj_ptr;
};
// make sure the factory defined above has its static variable defined.
template<typename TAG,typename OBJ_PTR, typename MEM_PTR, typename R,typename T1>
MEM_PTR MethodPtrCppCallbackFactory1<TAG,OBJ_PTR,MEM_PTR,R,T1>::m_mem_ptr;


// implement a factory for a 2-arg member method.
template <typename TAG, typename OBJ_PTR, typename MEM_PTR, typename R, typename T1, typename T2>
class MethodPtrCppCallbackFactory2 : public CppCallbackFactory2<R,T1,T2> {
public:
	MethodPtrCppCallbackFactory2 (OBJ_PTR obj_ptr)
		: m_obj_ptr (obj_ptr) {}
	~MethodPtrCppCallbackFactory2 () {}
	Callback<R,T1,T2> make_callback (void) const {
		return yans::make_callback (m_mem_ptr, m_obj_ptr);
	}
	static MEM_PTR m_mem_ptr;
private:
	OBJ_PTR m_obj_ptr;
};
// make sure the factory defined above has its static variable defined.
template<typename TAG,typename OBJ_PTR, typename MEM_PTR, typename R,typename T1,typename T2>
MEM_PTR MethodPtrCppCallbackFactory2<TAG,OBJ_PTR,MEM_PTR,R,T1,T2>::m_mem_ptr;


// helper function to for 1-arg member methods.
template <typename TAG, typename OBJ, typename R, typename T1>
void export_method_as_callback (char const *name, R (OBJ::*member) (T1))
{
	MethodPtrCppCallbackFactory1<TAG,OBJ *,R (OBJ::*) (T1),R,T1>::m_mem_ptr = member;
	class_<MethodPtrCppCallbackFactory1<TAG,OBJ*,R (OBJ::*) (T1),R,T1>, 
		bases<CppCallbackFactoryBase> > method (name, init<OBJ *> ());
}

// helper function for 2-arg member methods.
template <typename TAG, typename OBJ, typename R, typename T1,typename T2>
void export_method_as_callback (char const *name, R (OBJ::*member) (T1,T2))
{
	MethodPtrCppCallbackFactory2<TAG,OBJ *,R (OBJ::*) (T1),R,T1,T2>::m_mem_ptr = member;
	class_<MethodPtrCppCallbackFactory2<TAG,OBJ*,R (OBJ::*) (T1),R,T1,T2>, 
		bases<CppCallbackFactoryBase> > method (name, init<OBJ *> ());
}

// helper macro to call the export_method_as_callback functions defined above.
#define EXPORT_METHOD_AS_CALLBACK(class_name,method_name) \
{ \
	export_method_as_callback<struct class_name##_##method_name##_callback_tag> \
		(#class_name "_" #method_name "_callback_tag", &class_name::method_name); \
}

// extract a callback instance out of a factory instance.
template <typename R, typename T1>
Callback<R,T1> make_method_callback (CppCallbackFactoryBase *factory)
{
	return static_cast<CppCallbackFactory1<R,T1>*> (factory)->make_callback ();
}
template <typename R, typename T1,typename T2>
Callback<R,T1,T2> make_method_callback (CppCallbackFactoryBase *factory)
{
	return static_cast<CppCallbackFactory2<R,T1,T2>*> (factory)->make_callback ();
}

#endif /* METHOD_CALLBACK_TCC */
