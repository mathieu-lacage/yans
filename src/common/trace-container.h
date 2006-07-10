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
#include "callback-logger.h"
#include "callback.h"
#include "packet.h"
#include <list>
#include <string>

namespace yans {

class PacketLogger;
class TraceStream;

/**
 * \brief register every source of trace events
 *
 * Model authors use the TraceContainer class to register
 * their trace event sources. Model users use the TraceContainer
 * class to connect their trace event listeners to the
 * model trace event sources.
 *
 * TraceContainer can be used to register the following event sources:
 *   - yans::PacketLogger : can be connected to yans::PcapWriter
 *   - yans::TraceStream : can be connected to any std::ostream
 *   - yans::UiTracedVariable
 *   - yans::SiTracedVariable
 *   - yans::FTracedVariable
 *
 * The following sample code shows how you can:
 *   - create trace event sources
 *   - register the trace event sources in a trace container
 *   - set event sinks to each event source
 *  
 * \include samples/main-trace.cc
 */
class TraceContainer {
public:
	TraceContainer ();
	~TraceContainer ();

	/**
	 * \param name the name of the target event source
	 * \param callback the callback being connected to the target event source
	 * This method targets only event sources which are variables of any unsigned
	 * integer type.
	 */
	void set_ui_variable_callback (char const *name, 
				       Callback<void,uint64_t, uint64_t> callback);
	/**
	 * \param name the name of the target event source
	 * \param callback the callback being connected to the target event source
	 * This method targets only event sources which are variables of any signed
	 * integer type.
	 */
	void set_si_variable_callback (char const *name, Callback<void,int64_t, int64_t> callback);
	/**
	 * \param name the name of the target event source
	 * \param callback the callback being connected to the target event source
	 * This method targets only event sources which are variables of any double type.
	 */
	void set_f_variable_callback (char const *name, Callback<void,double, double> callback);
	/**
	 * \param name the name of the target event source
	 * \param callback the callback being connected to the target event source
	 * This method targets only event sources which are of type PacketLogger.
	 */
	void set_packet_logger_callback (char const *name, Callback<void,ConstPacketPtr> callback);
	/**
	 * \param name the name of the target event source
	 * \param callback the output stream being connected to the source trace stream
	 * This method targets only event sources which are of type TraceStream.
	 */
	void set_stream (char const *name, std::ostream *os);

	template <typename T1>
	void set_callback (char const *name, Callback<void,T1> callback);
	template <typename T1, typename T2>
	void set_callback (char const *name, Callback<void,T1,T2> callback);
	template <typename T1, typename T2, typename T3>
	void set_callback (char const *name, Callback<void,T1,T2,T3> callback);
	template <typename T1, typename T2, typename T3, typename T4>
	void set_callback (char const *name, Callback<void,T1,T2,T3,T4> callback);
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	void set_callback (char const *name, Callback<void,T1,T2,T3,T4,T5> callback);

	/**
	 * \param name the name of the registered event source
	 * \param var the event source being registered
	 * This method registers only event sources of type "unsigned integer".
	 */
	void register_ui_variable (char const *name, UiTracedVariableBase *var);
	/**
	 * \param name the name of the registered event source
	 * \param var the event source being registered
	 * This method registers only event sources of type "signed integer".
	 */
	void register_si_variable (char const *name, SiTracedVariableBase *var);
	/**
	 * \param name the name of the registered event source
	 * \param var the event source being registered
	 * This method registers only event sources of type "double".
	 */
	void register_f_variable (char const *name, FTracedVariableBase *var);
	/**
	 * \param name the name of the registered event source
	 * \param var the event source being registered
	 * This method registers only event sources of type PacketLogger.
	 */
	void register_packet_logger (char const *name, PacketLogger *logger);
	/**
	 * \param name the name of the registered event source
	 * \param var the event source being registered
	 * This method registers only event sources of type TraceStream.
	 */
	void register_stream (char const *name, TraceStream *stream);

	template <typename T1>
	void register_callback (char const *name, CallbackLogger<T1> *logger);
	template <typename T1, typename T2>
	void register_callback (char const *name, CallbackLogger<T1,T2> *logger);
	template <typename T1, typename T2, typename T3>
	void register_callback (char const *name, CallbackLogger<T1,T2,T3> *logger);
	template <typename T1, typename T2, typename T3, typename T4>
	void register_callback (char const *name, CallbackLogger<T1,T2,T3,T4> *logger);
	template <typename T1, typename T2,typename T3, typename T4, typename T5>
	void register_callback (char const *name, CallbackLogger<T1,T2,T3,T4,T5> *logger);

	/**
	 * Print the list of registered event sources.
	 */
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
	typedef std::list<std::pair<TraceStream *, std::string> > TraceStreamList;
	typedef std::list<std::pair<TraceStream *, std::string> >::iterator TraceStreamListI;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> > Callback1List;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> >::iterator Callback1ListI;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> > Callback2List;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> >::iterator Callback2ListI;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> > Callback3List;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> >::iterator Callback3ListI;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> > Callback4List;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> >::iterator Callback4ListI;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> > Callback5List;
	typedef std::list<std::pair<CallbackLoggerBase *, std::string> >::iterator Callback5ListI;

	UiList m_ui_list;
	SiList m_si_list;
	FList m_f_list;
	PacketLoggerList m_packet_logger_list;
	TraceStreamList m_trace_stream_list;
	Callback1List m_callback1_list;
	Callback2List m_callback2_list;
	Callback3List m_callback3_list;
	Callback4List m_callback4_list;
	Callback5List m_callback5_list;
};

}; // namespace yans

#ifndef NDEBUG
#include <cassert>
#endif

namespace yans {

template <typename T1>
void 
TraceContainer::set_callback (char const *name, Callback<void,T1> callback)
{
	for (Callback1ListI i = m_callback1_list.begin (); i != m_callback1_list.end (); i++) {
		if (i->second == name) {
			static_cast<CallbackLogger<T1> *> (i->first)->set_callback (callback);
		}
	}
#ifndef NDEBUG
	assert (false);
#endif
}
template <typename T1, typename T2>
void 
TraceContainer::set_callback (char const *name, Callback<void,T1,T2> callback)
{
	for (Callback2ListI i = m_callback2_list.begin (); i != m_callback2_list.end (); i++) {
		if (i->second == name) {
			static_cast<CallbackLogger<T1,T2> *> (i->first)->set_callback (callback);
		}
	}
#ifndef NDEBUG
	assert (false);
#endif
}
template <typename T1, typename T2, typename T3>
void 
TraceContainer::set_callback (char const *name, Callback<void,T1,T2,T3> callback)
{
	for (Callback3ListI i = m_callback3_list.begin (); i != m_callback3_list.end (); i++) {
		if (i->second == name) {
			static_cast<CallbackLogger<T1,T2,T3> *> (i->first)->set_callback (callback);
		}
	}
#ifndef NDEBUG
	assert (false);
#endif
}
template <typename T1, typename T2, typename T3, typename T4>
void 
TraceContainer::set_callback (char const *name, Callback<void,T1,T2,T3,T4> callback)
{
	for (Callback4ListI i = m_callback4_list.begin (); i != m_callback4_list.end (); i++) {
		if (i->second == name) {
			static_cast<CallbackLogger<T1,T2,T3,T4> *> (i->first)->set_callback (callback);
		}
	}
#ifndef NDEBUG
	assert (false);
#endif
}
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void 
TraceContainer::set_callback (char const *name, Callback<void,T1,T2,T3,T4,T5> callback)
{
	for (Callback5ListI i = m_callback5_list.begin (); i != m_callback5_list.end (); i++) {
		if (i->second == name) {
			static_cast<CallbackLogger<T1,T2,T3,T4,T5> *> (i->first)->set_callback (callback);
		}
	}
#ifndef NDEBUG
	assert (false);
#endif
}


template <typename T1>
void 
TraceContainer::register_callback (char const *name, CallbackLogger<T1> *logger)
{
#ifndef NDEBUG
	for (Callback1ListI i = m_callback1_list.begin (); i != m_callback1_list.end (); i++) {
		assert (i->second != name);
	}
#endif
	m_callback1_list.push_back (std::make_pair (logger, name));
}
template <typename T1, typename T2>
void 
TraceContainer::register_callback (char const *name, CallbackLogger<T1,T2> *logger)
{
#ifndef NDEBUG
	for (Callback2ListI i = m_callback2_list.begin (); i != m_callback2_list.end (); i++) {
		assert (i->second != name);
	}
#endif
	m_callback2_list.push_back (std::make_pair (logger, name));
}
template <typename T1, typename T2, typename T3>
void 
TraceContainer::register_callback (char const *name, CallbackLogger<T1,T2,T3> *logger)
{
#ifndef NDEBUG
	for (Callback3ListI i = m_callback3_list.begin (); i != m_callback3_list.end (); i++) {
		assert (i->second != name);
	}
#endif
	m_callback3_list.push_back (std::make_pair (logger, name));
}
template <typename T1, typename T2, typename T3, typename T4>
void 
TraceContainer::register_callback (char const *name, CallbackLogger<T1,T2,T3,T4> *logger)
{
#ifndef NDEBUG
	for (Callback4ListI i = m_callback4_list.begin (); i != m_callback4_list.end (); i++) {
		assert (i->second != name);
	}
#endif
	m_callback4_list.push_back (std::make_pair (logger, name));
}
template <typename T1, typename T2,typename T3, typename T4, typename T5>
void 
TraceContainer::register_callback (char const *name, CallbackLogger<T1,T2,T3,T4,T5> *logger)
{
#ifndef NDEBUG
	for (Callback5ListI i = m_callback5_list.begin (); i != m_callback5_list.end (); i++) {
		assert (i->second != name);
	}
#endif
	m_callback5_list.push_back (std::make_pair (logger, name));
}


}; // namespace yans

#endif /* TRACED_VARIABLE_CONTAINER_H */
