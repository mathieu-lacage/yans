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

#ifndef EVENT_TCC
#define EVENT_TCC

#include "event.h"
#include "event-environment-holder.tcc"

namespace yans {

template<typename T>
class EventCallback0 : public Event {
public:
	typedef void (T::*F)(void);

	EventCallback0 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (); 
		delete this;
	}
private:
	T* m_obj;
	F m_function;
};

template<typename T, typename T1>
class EventCallback1 : public Event {
public:
	typedef void (T::*F)(T1);

	EventCallback1 (T *obj, F function, T1 a1) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1.get ()); 
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	EventEnvironmentHolder<T1> m_a1;
};

template<typename T, typename T1, typename T2>
class EventCallback2 : public Event {
public:
	typedef void (T::*F)(T1, T2);

	EventCallback2 (T *obj, F function, T1 a1, T2 a2) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1),
		  m_a2 (a2)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1.get (), m_a2.get ());
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	EventEnvironmentHolder<T1> m_a1;
	EventEnvironmentHolder<T2> m_a2;
};

template<typename T, typename T1, typename T2, typename T3>
class EventCallback3 : public Event {
public:
	typedef void (T::*F)(T1, T2, T3);

	EventCallback3 (T *obj, F function, T1 a1, T2 a2, T3 a3) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1),
		  m_a2 (a2),
		  m_a3 (a3)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1.get (), m_a2.get (), m_a3.get ());
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	EventEnvironmentHolder<T1> m_a1;
	EventEnvironmentHolder<T2> m_a2;
	EventEnvironmentHolder<T3> m_a3;
};

template<typename T, typename T1, typename T2, typename T3, typename T4>
class EventCallback4 : public Event {
public:
	typedef void (T::*F)(T1, T2, T3, T4 a4);

	EventCallback4 (T *obj, F function, T1 a1, T2 a2, T3 a3, T4 a4) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1),
		  m_a2 (a2),
		  m_a3 (a3),
		  m_a4 (a4)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1.get (), m_a2.get (), m_a3.get (), m_a4.get ());
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	EventEnvironmentHolder<T1> m_a1;
	EventEnvironmentHolder<T2> m_a2;
	EventEnvironmentHolder<T3> m_a3;
	EventEnvironmentHolder<T4> m_a4;
};


template<typename T>
EventCallback0<T> *make_event(void (T::*f) (void), T* t) {
	return new EventCallback0<T>(t, f);
}
template<typename T, typename T1>
EventCallback1<T, T1> *make_event(void (T::*f) (T1), T* t, T1 a1) {
	return new EventCallback1<T, T1>(t, f, a1);
}
template<typename T, typename T1>
EventCallback1<T, T1 &> *make_event(void (T::*f) (T1 &), T* t, T1 &a1) {
	return new EventCallback1<T, T1 &>(t, f, a1);
}
template<typename T, typename T1, typename T2>
EventCallback2<T, T1, T2> *make_event(void (T::*f) (T1, T2), T* t, T1 a1, T2 a2) {
	return new EventCallback2<T, T1, T2>(t, f, a1, a2);
}
template<typename T, typename T1, typename T2, typename T3>
EventCallback3<T, T1, T2, T3> *make_event(void (T::*f) (T1, T2, T3), T* t, T1 a1, T2 a2, T3 a3) {
	return new EventCallback3<T, T1, T2, T3>(t, f, a1, a2, a3);
}
template<typename T, typename T1, typename T2, typename T3, typename T4>
EventCallback4<T, T1, T2, T3, T4> *make_event(void (T::*f) (T1, T2, T3, T4), T* t, T1 a1, T2 a2, T3 a3, T4 a4) {
	return new EventCallback4<T, T1, T2, T3, T4>(t, f, a1, a2, a3, a4);
}

}; // namespace yans

#endif /* EVENT_TCC */
