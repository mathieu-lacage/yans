/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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

#include "traced-variable-container.h"
#include "traced-variable.tcc"
#include <utility>

namespace yans {

TracedVariableContainer::TracedVariableContainer ()
{}
TracedVariableContainer::~TracedVariableContainer ()
{
	m_ui_list.erase (m_ui_list.begin (), m_ui_list.end ());
	m_si_list.erase (m_si_list.begin (), m_si_list.end ());
	m_f_list.erase (m_f_list.begin (), m_f_list.end ());
}

void 
TracedVariableContainer::set_ui_callback (char const *name, Callback<void (uint64_t, uint64_t)> *callback)
{
	for (UiListI i = m_ui_list.begin (); i != m_ui_list.end (); i++) {
		if ((*i).second == name) {
			(*i).first->set_callback (callback);
		}
	}
	assert (false);
}
void 
TracedVariableContainer::set_si_callback (char const *name, Callback<void (int64_t, int64_t)> *callback)
{}
void 
TracedVariableContainer::set_f_callback (char const *name, Callback<void (double, double)> *callback)
{}

void 
TracedVariableContainer::register_ui_variable (UiTracedVariableBase *var, char const *name)
{
	// ensure unicity
	for (UiListI i = m_ui_list.begin (); i != m_ui_list.end (); i++) {
		assert ((*i).second != name);
	}
	m_ui_list.push_back (std::make_pair (var, name));
}
void 
TracedVariableContainer::register_si_variable (SiTracedVariableBase *var, char const *name)
{}
void 
TracedVariableContainer::register_f_variable (FTracedVariableBase *var, char const *name)
{}


}; // namespace yans
