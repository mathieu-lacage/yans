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

#ifndef STATIC_EVENT_TCC
#define STATIC_EVENT_TCC

#include "static-event.h"

namespace yans {

template<typename T>
class StaticEventCallback0 : public StaticEvent {
public:
	typedef void (T::*F)(void);

	StaticEventCallback0 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (); 
	}
private:
	T* m_obj;
	F m_function;
};

template<typename T, typename T1>
class StaticEventCallback1 : public StaticEvent {
public:
	typedef void (T::*F)(T1);

	StaticEventCallback1 (T *obj, F function, T1 a1) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1); 
	}
private:
	T* m_obj;
	F m_function;
	T1 m_a1;
};

template<typename T, typename T1, typename T2>
class StaticEventCallback2 : public StaticEvent {
public:
	typedef void (T::*F)(T1, T2);

	StaticEventCallback2 (T *obj, F function, T1 a1, T2 a2) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1),
		  m_a2 (a2)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1, m_a2);
	}
private:
	T* m_obj;
	F m_function;
	T1 m_a1;
	T2 m_a2;
};


template<typename T>
StaticEventCallback0<T> *make_static_event(void (T::*f) (void), T* t) {
	return new StaticEventCallback0<T>(t, f);
}
template<typename T, typename T1>
StaticEventCallback1<T, T1> *make_static_event(void (T::*f) (T1), T* t, T1 a1) {
	return new StaticEventCallback1<T, T1>(t, f, a1);
}
template<typename T, typename T1, typename T2>
StaticEventCallback2<T, T1, T2> *make__static_event(void (T::*f) (T1, T2), T* t, T1 a1, T2 a2) {
	return new StaticEventCallback2<T, T1, T2>(t, f, a1, a2);
}

}; // namespace yans

#endif /* STATIC_EVENT_TCC */