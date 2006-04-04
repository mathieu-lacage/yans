/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef CALLBACK_H
#define CALLBACK_H

namespace yans {

/***
 * This code was originally written based on the techniques 
 * described in http://www.codeproject.com/cpp/TTLFunction.asp
 * It was subsequently rewritten to follow the architecture
 * outlined in "Modern C++ Design" by Andrei Alexandrescu in 
 * chapter 5, "Generalized Functors".
 */
class empty {};
class CallbackBase {};

// declare the CallbackImpl class
template <typename R, typename T1, typename T2, typename T3, typename T4>
class CallbackImpl;
// define CallbackImpl for 0 params
template <typename R>
class CallbackImpl<R,empty,empty,empty,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (void) = 0;
	virtual CallbackImpl *copy (void) const = 0;
};
// define CallbackImpl for 1 params
template <typename R, typename T1>
class CallbackImpl<R,T1,empty,empty,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1) = 0;
	virtual CallbackImpl *copy (void) const = 0;
};
// define CallbackImpl for 2 params
template <typename R, typename T1, typename T2>
class CallbackImpl<R,T1,T2,empty,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1, T2) = 0;
	virtual CallbackImpl *copy (void) const = 0;
};
// define CallbackImpl for 3 params
template <typename R, typename T1, typename T2, typename T3>
class CallbackImpl<R,T1,T2,T3,empty> {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1, T2, T3) = 0;
	virtual CallbackImpl *copy (void) const = 0;
};
// define CallbackImpl for 4 params
template <typename R, typename T1, typename T2, typename T3, typename T4>
class CallbackImpl {
public:
	virtual ~CallbackImpl () {}
	virtual R operator() (T1, T2, T3, T4) = 0;
	virtual CallbackImpl *copy (void) const = 0;
};


// an impl for Functors:
template <typename T, typename R, typename T1, typename T2, typename T3, typename T4>
class FunctorCallbackImpl : public CallbackImpl<R,T1,T2,T3,T4> {
public:
	FunctorCallbackImpl (T const &functor)
		: m_functor (functor) {}
	virtual ~FunctorCallbackImpl () {}
	virtual FunctorCallbackImpl *copy (void) const {
		return new FunctorCallbackImpl (*this);
	}
	R operator() (void) {
		return m_functor ();
	}
	R operator() (T1 a1) {
		return m_functor (a1);
	}
	R operator() (T1 a1,T2 a2) {
		return m_functor (a1,a2);
	}
	R operator() (T1 a1,T2 a2,T3 a3) {
		return m_functor (a1,a2,a3);
	}
	R operator() (T1 a1,T2 a2,T3 a3,T4 a4) {
		return m_functor (a1,a2,a3,a4);
	}
private:
	T m_functor;
};

// an impl for pointer to member functions
template <typename OBJ_PTR, typename MEM_PTR, typename R, typename T1, typename T2, typename T3, typename T4>
class MemPtrCallbackImpl : public CallbackImpl<R,T1,T2,T3,T4> {
public:
	MemPtrCallbackImpl (OBJ_PTR const&obj_ptr, MEM_PTR mem_ptr)
		: m_obj_ptr (obj_ptr), m_mem_ptr (mem_ptr) {}
	virtual ~MemPtrCallbackImpl () {}
	virtual MemPtrCallbackImpl *copy (void) const {
		return new MemPtrCallbackImpl (*this);
	}
	R operator() (void) {
		return ((*m_obj_ptr).*m_mem_ptr) ();
	}
	R operator() (T1 a1) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1);
	}
	R operator() (T1 a1,T2 a2) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1,a2);
	}
	R operator() (T1 a1,T2 a2,T3 a3) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1,a2,a3);
	}
	R operator() (T1 a1,T2 a2,T3 a3,T4 a4) {
		return ((*m_obj_ptr).*m_mem_ptr) (a1,a2,a3,a4);
	}
private:
	OBJ_PTR const m_obj_ptr;
	MEM_PTR m_mem_ptr;
};




// declare and define Callback class
template<typename R, 
	 typename T1 = empty, typename T2 = empty, 
	 typename T3 = empty, typename T4 = empty>
class Callback : public CallbackBase {
public:
	template <typename FUNCTOR>
	Callback (FUNCTOR const &functor) 
		: m_impl (new FunctorCallbackImpl<FUNCTOR,R,T1,T2,T3,T4> (functor))
	{}

	template <typename OBJ_PTR, typename MEM_PTR>
	Callback (OBJ_PTR const &obj_ptr, MEM_PTR mem_ptr)
		: m_impl (new MemPtrCallbackImpl<OBJ_PTR,MEM_PTR,R,T1,T2,T3,T4> (obj_ptr, mem_ptr))
	{}

	Callback () : m_impl (0) {}
	~Callback () {delete m_impl;}
	Callback (Callback const&o) : m_impl (o.m_impl->copy ()) {}
	Callback &operator = (Callback const&o) {
		delete m_impl;
		m_impl = o.m_impl->copy ();
		return *this;
	}

	R operator() (void) {
		return (*m_impl) ();
	}
	R operator() (T1 a1) {
		return (*m_impl) (a1);
	}
	R operator() (T1 a1, T2 a2) {
		return (*m_impl) (a1,a2);
	}
	R operator() (T1 a1, T2 a2, T3 a3) {
		return (*m_impl) (a1,a2,a3);
	}
	R operator() (T1 a1, T2 a2, T3 a3, T4 a4) {
		return (*m_impl) (a1,a2,a3,a4);
	}
private:
	CallbackImpl<R,T1,T2,T3,T4> *m_impl;
};

template <typename OBJ_PTR, typename MEM_PTR>
Callback<void> make_callback (MEM_PTR mem_ptr, OBJ_PTR obj_ptr) {
	return Callback<void> (obj_ptr, mem_ptr);
}

}; // namespace yans


#endif /* CALLBACK_H */
