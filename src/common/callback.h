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
class FunctorCallerBase0
{
	virtual ~FunctorCallerBase0 () {}
	virtual R operator() (void) = 0;
};

template< typename F, typename R >
class FunctorCaller0 : FunctorCallerBase0<R>
{
	FunctorCaller0( F f ) : m_functor (f) {}
	virtual R operator() (void) { return m_functor (); }
private:
	F m_functor;
};

template< typename R, typename T1 = void, typename T2 = void>
class Callback;


template< typename R, typename T1, typename T2 >
class Callback
{
	typedef void (T::*F)(void);
	typedef FunctorCallerBase0<R> caller;
    
	template< typename F >
	Callback (F f) : m_functor (0) {
		typedef FunctorCaller0<F,R> caller_spec;
		m_functor = new caller_spec(f);
	}
	R operator() (void) { 
		return (*m_functor)(); 
	}

private:
	caller *m_functor;
};


template<typename T>
Callback<T> *make_callback(void (T::*f) (void), T* t) {
	return new Callback<T>(t, f);
}

#endif /* CALLBACK_H */
