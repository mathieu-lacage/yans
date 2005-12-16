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

#ifndef CALLBACK_EVENT_TCC
#define CALLBACK_EVENT_TCC

#include "event.h"
#include "event.tcc"

/***
 * This code is unreadable. It was inspired by the techniques described in:
 * http://www.codeproject.com/cpp/TTLFunction.asp
 */

class empty {};

template<typename T1 = empty, typename T2 = empty>
class CallbackEvent;


template<>
class CallbackEvent<void ()> {
public:
	virtual ~Callback () {}
	virtual void invoke_later (void);
};

template<typename T1>
class CallbackEvent<void (T1)> {
 public:
	virtual ~Callback () {}
	virtual void invoke_later (T1) = 0;
};

template<typename T1, typename T2>
class CallbackEvent<void (T1, T2)> {
 public:
	virtual ~CallbackEvent () {}
	virtual void invoke_later (T1, T2) = 0;
};



template<typename T>
class CallbackEvent0 : public CallbackEvent<void (void)> {
public:
	typedef void (T::*F) (void);

	CallbackEvent0 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function) 
	{}
	virtual void invoke_later (void) {
		Event *ev = make_event (m_function, m_obj);
		Simulator::insert_later (ev);
	}
private:
	T *m_obj;
	F m_function;
};

template<typename T, typename T1>
class CallbackEvent1 : public CallbackEvent<void (T1)> {
public:
	typedef void (T::*F) (T1);

	CallbackEvent1 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function) 
	{}
	virtual void invoke_later (T1 a1) {
		Event *ev = make_event (m_function, m_obj, a1);
		Simulator::insert_later (ev);
	}
private:
	T *m_obj;
	F m_function;
};

template<typename T, typename T1, typename T2>
class CallbackEvent2 : public CallbackEvent<void (T1, T2)> {
public:
	typedef void (T::*F) (T1, T2);

	CallbackEvent2 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function) 
	{}
	virtual void invoke_later (T1 a1, T2 a2) {
		Event *ev = make_event (m_function, m_obj, a1, a2);
		Simulator::insert_later (ev);
	}
private:
	T *m_obj;
	F m_function;
};


template<typename T>
CallbackEvent0<T> *make_callback_event (void (T::*f) (void), T* t) {
	return new CallbackEvent0<T>(t, f);
}
template<typename T, typename T1>
CallbackEvent1<T, T1> *make_callback_event (void (T::*f) (T1), T* t) {
	return new CallbackEvent1<T, T1>(t, f);
}
template<typename T, typename T1, typename T2>
CallbackEvent2<T, T1, T2> *make_callback_event (void (T::*f) (T1, T2), T* t) {
	return new CallbackEvent2<T, T1, T2>(t, f);
}

#endif /* CALLBACK_EVENT_TCC */
