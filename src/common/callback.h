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

class CallbackBase {
public:
	virtual void operator () (void) = 0;
};

template<typename T>
class Callback : public CallbackBase {
public:
	typedef void (T::*F)(void);

	Callback(T *t, F f) : t_(t), f_(f) { }
	virtual void operator () (void)
	{ (t_->*f_)(); delete this;}
private:
	T* t_;
	F f_;
};

template<typename T>
Callback<T> *make_callback(void (T::*f) (void), T* t) {
	return new Callback<T>(t, f);
}

#endif /* CALLBACK_H */
