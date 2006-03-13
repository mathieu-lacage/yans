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

#ifndef TRACE_CONTAINER_H
#define TRACE_CONTAINER_H

#include "ui-traced-variable.tcc"
#include "si-traced-variable.tcc"
#include "f-traced-variable.tcc"
#include "callback.tcc"
#include <list>
#include <string>

namespace yans {

class Packet;
class PacketLogger;

class TraceContainer {
public:
	TraceContainer ();
	~TraceContainer ();

	void set_ui_variable_callback (char const *name, Callback<void (uint64_t, uint64_t)> *callback);
	void set_si_variable_callback (char const *name, Callback<void (int64_t, int64_t)> *callback);
	void set_f_variable_callback (char const *name, Callback<void (double, double)> *callback);
	void set_packet_logger_callback (char const *name, Callback<void (Packet const*)> *callback);

	void register_ui_variable (char const *name, UiTracedVariableBase *var);
	void register_si_variable (char const *name, SiTracedVariableBase *var);
	void register_f_variable (char const *name, FTracedVariableBase *var);
	void register_packet_logger (char const *name, PacketLogger *logger);

	void print_debug (void);
private:
	typedef std::list<std::pair<UiTracedVariableBase *, std::string> > UiList;
	typedef std::list<std::pair<UiTracedVariableBase *, std::string> >::iterator UiListI;
	typedef std::list<std::pair<SiTracedVariableBase *, std::string> > SiList;
	typedef std::list<std::pair<SiTracedVariableBase *, std::string> >::iterator SiListI;
	typedef std::list<std::pair<FTracedVariableBase *, std::string> > FList;
	typedef std::list<std::pair<FTracedVariableBase *, std::string> >::iterator FListI;
	typedef std::list<std::pair<PacketLogger *, std::string> > PacketLoggerList;
	typedef std::list<std::pair<PacketLogger *, std::string> >::iterator PacketLoggerListI;

	UiList m_ui_list;
	SiList m_si_list;
	FList m_f_list;
	PacketLoggerList m_packet_logger_list;
};

}; // namespace yans

#endif /* TRACED_VARIABLE_CONTAINER_H */
