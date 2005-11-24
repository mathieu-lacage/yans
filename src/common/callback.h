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

template< typename R, typename T1 = void, typename T2 = void>
class Callback;


template< typename R>
class Callback<R> {
public:
	virtual ~Callback () {}
	virtual R operator() (void) = 0;
};

template< typename R, typename T1>
class Callback<R (T1)> {
 public:
	virtual ~Callback () {}
	virtual R operator() (T1) = 0;
};

template< typename R, typename T1, typename T2>
class Callback<R (T1, T2)> {
 public:
	virtual ~Callback () {}
	virtual R operator() (T1, T2) = 0;
};



template<typename T, typename R>
class Callback0 : public Callback<R> {
public:
	typedef R (T::*F) (void);

	Callback0 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function) 
	{}
	virtual R operator() (void) {
		(m_obj->*m_function) ();
	}
private:
	T *m_obj;
	F m_function;
};

template<typename T, typename R, typename T1>
class Callback1 : public Callback<R (T1)> {
public:
	typedef R (T::*F) (T1);

	Callback1 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function) 
	{}
	virtual R operator() (T1 a1) {
		(m_obj->*m_function) (a1);
	}
private:
	T *m_obj;
	F m_function;
};

template<typename T, typename R, typename T1, typename T2>
class Callback2 : public Callback<R (T1, T2)> {
public:
	typedef R (T::*F) (T1, T2);

	Callback2 (T *obj, F function) 
		: m_obj (obj), 
		  m_function (function) 
	{}
	virtual R operator() (T1 a1, T2 a2) {
		(m_obj->*m_function) (a1, a2);
	}
private:
	T *m_obj;
	F m_function;
};


template<typename T, typename R>
Callback0<T, R> *make_callback(R (T::*f) (void), T* t) {
	return new Callback0<T, R>(t, f);
}
template<typename T, typename R, typename T1>
Callback1<T, R, T1> *make_callback(R (T::*f) (T1), T* t) {
	return new Callback1<T, R, T1>(t, f);
}
template<typename T, typename R, typename T1, typename T2>
Callback2<T, R, T1, T2> *make_callback(R (T::*f) (T1, T2), T* t) {
	return new Callback2<T, R, T1, T2>(t, f);
}


#endif /* CALLBACK_H */
