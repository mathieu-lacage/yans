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
#include "callback.h"
#include "udp-source.h"
#include "traffic-analyser.h"
#include "throughput-printer.h"
#include "id-factory.h"


namespace yapns {

CallbackVoidPacket::CallbackVoidPacket (::Remote::CallbackVoidPacket_ptr remote)
{
	m_remote = ::Remote::CallbackVoidPacket::_duplicate (remote);
}
CallbackVoidPacket::CallbackVoidPacket (CallbackVoidPacket const &o)
{
	m_remote = ::Remote::CallbackVoidPacket::_duplicate (o.m_remote);
}
CallbackVoidPacket::~CallbackVoidPacket ()
{
	CORBA::release (m_remote);
}
::Remote::CallbackVoidPacket_ptr 
CallbackVoidPacket::peek_remote (void)
{
	return m_remote;
}

CallbackVoidPacket
make_callback (void (UdpSource::*) (Packet *), UdpSource *self)
{
	::Remote::CallbackVoidPacket_ptr remote = 
		self->peek_remote ()->create_send_callback (IdFactory::get_next ());
	CallbackVoidPacket callback = CallbackVoidPacket (remote);
	return callback;
}

CallbackVoidPacket
make_callback (void (TrafficAnalyser::*) (Packet *), TrafficAnalyser *self)
{
	::Remote::CallbackVoidPacket_ptr remote = 
		self->peek_remote ()->create_receive_callback (IdFactory::get_next ());
	CallbackVoidPacket callback = CallbackVoidPacket (remote);
	return callback;
}

CallbackVoidPacket
make_callback (void (ThroughputPrinter::*ptr) (Packet *), ThroughputPrinter *self)
{
	::Remote::CallbackVoidPacket_ptr remote = 
		self->peek_remote ()->create_receive_callback (IdFactory::get_next ());
	CallbackVoidPacket callback = CallbackVoidPacket (remote);
	return callback;
}




}; // namespace yapns
