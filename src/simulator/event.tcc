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

namespace yans {

template<typename T>
class EventMemberImpl0 : public Event {
public:
	typedef void (T::*F)(void);

	EventMemberImpl0 (T *obj, F function) 
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
class EventMemberImpl1 : public Event {
public:
	typedef void (T::*F)(T1);

	EventMemberImpl1 (T *obj, F function, T1 a1) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1);
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	T1 m_a1;
};

template<typename T, typename T1, typename T2>
class EventMemberImpl2 : public Event {
public:
	typedef void (T::*F)(T1, T2);

	EventMemberImpl2 (T *obj, F function, T1 a1, T2 a2) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1),
		  m_a2 (a2)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1, m_a2);
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	T1 m_a1;
	T2 m_a2;
};

template<typename T, typename T1, typename T2, typename T3>
class EventMemberImpl3 : public Event {
public:
	typedef void (T::*F)(T1, T2, T3);

	EventMemberImpl3 (T *obj, F function, T1 a1, T2 a2, T3 a3) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1),
		  m_a2 (a2),
		  m_a3 (a3)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1, m_a2, m_a3);
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	T1 m_a1;
	T2 m_a2;
	T3 m_a3;
};

template<typename T, typename T1, typename T2, typename T3, typename T4>
class EventMemberImpl4 : public Event {
public:
	typedef void (T::*F)(T1, T2, T3, T4);

	EventMemberImpl4 (T *obj, F function, T1 a1, T2 a2, T3 a3, T4 a4) 
		: m_obj (obj), 
		  m_function (function),
		  m_a1 (a1),
		  m_a2 (a2),
		  m_a3 (a3),
		  m_a4 (a4)
	{ }
	virtual void notify (void) { 
		(m_obj->*m_function) (m_a1, m_a2, m_a3, m_a4);
		delete this;
	}
private:
	T* m_obj;
	F m_function;
	T1 m_a1;
	T2 m_a2;
	T3 m_a3;
	T4 m_a4;
};


template<typename T>
EventMemberImpl0<T> *make_event(void (T::*f) (void), T* t) {
	return new EventMemberImpl0<T>(t, f);
}
template<typename T, typename T1>
EventMemberImpl1<T, T1> *make_event(void (T::*f) (T1), T* t, T1 a1) {
	return new EventMemberImpl1<T, T1>(t, f, a1);
}
template<typename T, typename T1, typename T2>
EventMemberImpl2<T, T1, T2> *make_event(void (T::*f) (T1, T2), T* t, T1 a1, T2 a2) {
	return new EventMemberImpl2<T, T1, T2>(t, f, a1, a2);
}
template<typename T, typename T1, typename T2, typename T3>
EventMemberImpl3<T, T1, T2, T3> *make_event(void (T::*f) (T1, T2, T3), T* t, T1 a1, T2 a2, T3 a3) {
	return new EventMemberImpl3<T, T1, T2, T3>(t, f, a1, a2, a3);
}
template<typename T, typename T1, typename T2, typename T3, typename T4>
EventMemberImpl4<T, T1, T2, T3, T4> *make_event(void (T::*f) (T1, T2, T3, T4), T* t, T1 a1, T2 a2, T3 a3, T4 a4) {
	return new EventMemberImpl4<T, T1, T2, T3, T4>(t, f, a1, a2, a3, a4);
}

class EventFunctionImpl0 : public Event {
public:
	typedef void (*F)(void);

	EventFunctionImpl0 (F function) 
		: m_function (function)
	{ }
	virtual void notify (void) { 
		(*m_function) (); 
		delete this;
	}
private:
	F m_function;
};

template<typename T1>
class EventFunctionImpl1 : public Event {
public:
	typedef void (*F)(T1);

	EventFunctionImpl1 (F function, T1 a1) 
		: m_function (function),
		  m_a1 (a1)
	{ }
	virtual void notify (void) { 
		(*m_function) (m_a1);
		delete this;
	}
private:
	F m_function;
	T1 m_a1;
};

template<typename T1, typename T2>
class EventFunctionImpl2 : public Event {
public:
	typedef void (*F)(T1, T2);

	EventFunctionImpl2 (F function, T1 a1, T2 a2) 
		: m_function (function),
		  m_a1 (a1),
		  m_a2 (a2)
	{ }
	virtual void notify (void) { 
		(*m_function) (m_a1, m_a2);
		delete this;
	}
private:
	F m_function;
	T1 m_a1;
	T2 m_a2;
};

template<typename T1, typename T2, typename T3>
class EventFunctionImpl3 : public Event {
public:
	typedef void (*F)(T1, T2, T3);

	EventFunctionImpl3 (F function, T1 a1, T2 a2, T3 a3) 
		: m_function (function),
		  m_a1 (a1),
		  m_a2 (a2),
		  m_a3 (a3)
	{ }
	virtual void notify (void) { 
		(*m_function) (m_a1, m_a2, m_a3);
		delete this;
	}
private:
	F m_function;
	T1 m_a1;
	T2 m_a2;
	T3 m_a3;
};

template<typename T1, typename T2, typename T3, typename T4>
class EventFunctionImpl4 : public Event {
public:
	typedef void (*F)(T1, T2, T3, T4);

	EventFunctionImpl4 (F function, T1 a1, T2 a2, T3 a3, T4 a4) 
		: m_function (function),
		  m_a1 (a1),
		  m_a2 (a2),
		  m_a3 (a3),
		  m_a4 (a4)
	{ }
	virtual void notify (void) { 
		(*m_function) (m_a1, m_a2, m_a3, m_a4);
		delete this;
	}
private:
	F m_function;
	T1 m_a1;
	T2 m_a2;
	T3 m_a3;
	T4 m_a4;
};


template<typename T>
EventFunctionImpl0 *make_event(void (*f) (void)) {
	return new EventFunctionImpl0 (f);
}
template<typename T1>
EventFunctionImpl1<T1> *make_event(void (*f) (T1), T1 a1) {
	return new EventFunctionImpl1<T1>(f, a1);
}
template<typename T1, typename T2>
EventFunctionImpl2<T1, T2> *make_event(void (*f) (T1, T2), T1 a1, T2 a2) {
	return new EventFunctionImpl2<T1, T2>(f, a1, a2);
}
template<typename T1, typename T2, typename T3>
EventFunctionImpl3<T1, T2, T3> *make_event(void (*f) (T1, T2, T3), T1 a1, T2 a2, T3 a3) {
	return new EventFunctionImpl3<T1, T2, T3>(f, a1, a2, a3);
}
template<typename T1, typename T2, typename T3, typename T4>
EventFunctionImpl4<T1, T2, T3, T4> *make_event(void (*f) (T1, T2, T3, T4), T1 a1, T2 a2, T3 a3, T4 a4) {
	return new EventFunctionImpl4<T1, T2, T3, T4>(f, a1, a2, a3, a4);
}


}; // namespace yans

#endif /* EVENT_TCC */
