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

#include "trace-container.h"
#include "packet-logger.h"
#include <utility>

namespace yans {

TraceContainer::TraceContainer ()
{}
TraceContainer::~TraceContainer ()
{
	m_ui_list.erase (m_ui_list.begin (), m_ui_list.end ());
	m_si_list.erase (m_si_list.begin (), m_si_list.end ());
	m_f_list.erase (m_f_list.begin (), m_f_list.end ());
}

void 
TraceContainer::set_ui_variable_callback (char const *name, Callback<void (uint64_t, uint64_t)> *callback)
{
	for (UiListI i = m_ui_list.begin (); i != m_ui_list.end (); i++) {
		if ((*i).second == name) {
			(*i).first->set_callback (callback);
			return;
		}
	}
	assert (false);
}
void 
TraceContainer::set_si_variable_callback (char const *name, Callback<void (int64_t, int64_t)> *callback)
{
	for (SiListI i = m_si_list.begin (); i != m_si_list.end (); i++) {
		if ((*i).second == name) {
			(*i).first->set_callback (callback);
			return;
		}
	}
	assert (false);
}
void 
TraceContainer::set_f_variable_callback (char const *name, Callback<void (double, double)> *callback)
{
	assert (false);
}
void 
TraceContainer::set_packet_logger_callback (char const *name, Callback<void (Packet *)> *callback)
{
	for (PacketLoggerListI i = m_packet_logger_list.begin (); i != m_packet_logger_list.end (); i++) {
		if ((*i).second == name) {
			(*i).first->set_callback (callback);
			return;
		}
	}
	assert (false);	
}

void 
TraceContainer::register_ui_variable (char const *name, UiTracedVariableBase *var)
{
	// ensure unicity
	for (UiListI i = m_ui_list.begin (); i != m_ui_list.end (); i++) {
		assert ((*i).second != name);
	}
	m_ui_list.push_back (std::make_pair (var, name));
}
void 
TraceContainer::register_si_variable (char const *name, SiTracedVariableBase *var)
{
	// ensure unicity
	for (SiListI i = m_si_list.begin (); i != m_si_list.end (); i++) {
		assert ((*i).second != name);
	}
	m_si_list.push_back (std::make_pair (var, name));
}
void 
TraceContainer::register_f_variable (char const *name, FTracedVariableBase *var)
{
	assert (false);
}

void 
TraceContainer::register_packet_logger (char const *name, PacketLogger *logger)
{
	// ensure unicity
	for (PacketLoggerListI i = m_packet_logger_list.begin (); i != m_packet_logger_list.end (); i++) {
		assert ((*i).second != name);
	}
	m_packet_logger_list.push_back (std::make_pair (logger, name));
}



}; // namespace yans
