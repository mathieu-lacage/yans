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

template<typename R>
class CallbackBase0 {
public:
	virtual ~CallbackBase0 () {}
	virtual R operator() (void) = 0;
};

template<typename T, typename R>
class Callback0 : public CallbackBase0 {
public:
	typedef void (T::*F) (void);

	CallbackBase0 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function) 
	{}
	virtual ~CallbackBase0 () {}
	virtual R operator() (void) {
		(m_obj->*m_function) ();
	}
private:
	T *m_obj;
	F m_function;
};


template< typename R, typename T1 = void, typename T2 = void>
class Callback;


template< typename R, typename T1, typename T2>
class Callback
{
	typedef void (T::*F)(void);
    
	template< typename T>
	Callback (T * obj, F function) 
		: m_callback (obj, function) {}
	R operator() (void) { 
		return m_callback (); 
	}

private:
	Callback0<R> m_callback;
};


template<typename T>
Callback0<T,R> *make_callback(R (T::*f) (void), T* t) {
	return new Callback0<T, R>(t, f);
}

#endif /* CALLBACK_H */
