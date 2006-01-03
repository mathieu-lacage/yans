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

#include <boost/python.hpp>
#include "thread.h"

using namespace yans;
using namespace boost::python;

struct ThreadWrap : Thread, wrapper<Thread> {
	ThreadWrap (PyObject *self, char const *name)
		: Thread (name), 
		  m_self (self) 
	{
		Py_INCREF(m_self);
	}
	ThreadWrap::~ThreadWrap() 
	{
		Py_DECREF(m_self);
	}
private:
	virtual void run (void) {
		call_method<void>(m_self, "run");
	}
	PyObject *m_self;
};

/* specialize has_back_reference for ThreadWrap to make
   sure a PyObject* is passed to its constructor
*/
namespace boost { namespace python {
	template <>
	struct has_back_reference<ThreadWrap>
		: mpl::true_ {};
}}


void
export_thread (void)
{
	class_<ThreadWrap, boost::noncopyable> thread ("Thread", init<char const *> ());
	thread.def ("yield", &Thread::yield);
	thread.def ("sleep_s", &Thread::sleep_s);
	thread.def ("sleep_us", &Thread::sleep_us);
	thread.def ("time_s", &Thread::time_s);
}
