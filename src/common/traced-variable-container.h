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

#ifndef TRACED_VARIABLE_CONTAINER_H
#define TRACED_VARIABLE_CONTAINER_H

#include "traced-variable.tcc"
#include "callback.tcc"
#include <list>
#include <string>

namespace yans {

class TracedVariableContainer {
public:
	TracedVariableContainer ();
	~TracedVariableContainer ();

	void set_ui_callback (char const *name, Callback<void (uint64_t, uint64_t)> *callback);
	void set_si_callback (char const *name, Callback<void (int64_t, int64_t)> *callback);
	void set_f_callback (char const *name, Callback<void (double, double)> *callback);

	void register_ui_variable (UiTracedVariableBase *var, char const *name);
	void register_si_variable (SiTracedVariableBase *var, char const *name);
	void register_f_variable (FTracedVariableBase *var, char const *name);
private:
	typedef std::list<std::pair<UiTracedVariableBase *, std::string> > UiList;
	typedef std::list<std::pair<UiTracedVariableBase *, std::string> >::iterator UiListI;
	typedef std::list<std::pair<SiTracedVariableBase *, std::string> > SiList;
	typedef std::list<std::pair<SiTracedVariableBase *, std::string> >::iterator SiListI;
	typedef std::list<std::pair<FTracedVariableBase *, std::string> > FList;
	typedef std::list<std::pair<FTracedVariableBase *, std::string> >::iterator FListI;

	UiList m_ui_list;
	SiList m_si_list;
	FList m_f_list;
};

}; // namespace yans

#endif /* TRACED_VARIABLE_CONTAINER_H */
