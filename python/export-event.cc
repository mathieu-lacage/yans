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
#include "event.h"
#include "event-wrap.h"
#include <Python.h>

using namespace yans;
using namespace boost::python;

EventWrap::EventWrap (PyObject* self_)
	: m_self(self_)
{
	Py_INCREF(m_self);
}
EventWrap::~EventWrap()
{
	Py_DECREF(m_self);
}


void 
EventWrap::notify (void) 
{
	call_method<void>(m_self, "notify");
	//delete this;
}

// specialize has_back_reference for EventWrap to make
// sure a PyObject* is passed to its constructor
namespace boost { namespace python {
	template <>
	struct has_back_reference<EventWrap>
		: mpl::true_ {};
}}

void 
export_event (void)
{
	class_<EventWrap, std::auto_ptr<EventWrap>, boost::noncopyable> event ("Event");
	event.def ("notify", pure_virtual(&Event::notify));
}
