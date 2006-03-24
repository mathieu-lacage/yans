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
#include "function-holder.h"
#include "yans/simulator.h"
#include "yans/event.h"

using namespace yans;
using namespace boost::python;

class FunctionHolderEvent : public Event {
public:
	FunctionHolderEvent (FunctionHolder holder)
		: m_holder (holder) {} 
	virtual void notify (void) {
		object function = m_holder.get_function ();
		object context = m_holder.get_context ();
		function (context);
		delete this;
	}
private:
	FunctionHolder m_holder;
};


void 
simu_insert_in_s (double delta, FunctionHolder holder)
{
	Event *ev = new FunctionHolderEvent (holder);
	Simulator::insert_in_s (delta, ev);
}

void 
simu_insert_in_us (uint64_t delta, FunctionHolder holder)
{
	Event *ev = new FunctionHolderEvent (holder);
	Simulator::insert_in_us (delta, ev);
}

void 
simu_insert_at_s (double at, FunctionHolder holder)
{
	Event *ev = new FunctionHolderEvent (holder);
	Simulator::insert_at_s (at, ev);
}

void 
simu_insert_at_us (uint64_t at, FunctionHolder holder)
{
	Event *ev = new FunctionHolderEvent (holder);
	Simulator::insert_at_us (at, ev);
}

void 
simu_insert_later (FunctionHolder holder)
{
	Event *ev = new FunctionHolderEvent (holder);
	Simulator::insert_later (ev);
}

void 
export_simulator (void)
{
	def ("run", Simulator::run);
	def ("stop", Simulator::stop);

	def ("destroy", Simulator::destroy);

	def ("insert_in_s", simu_insert_in_s);
	def ("insert_in_us", simu_insert_in_us);

	def ("insert_at_s", simu_insert_at_s);
	def ("insert_at_us", simu_insert_at_us);

	def ("now_s", Simulator::now_s);
	def ("now_us", Simulator::now_us);

	def ("insert_later", simu_insert_later);
}
