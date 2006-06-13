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
#ifndef YAPNS_CALLBACK_H
#define YAPNS_CALLBACK_H

#include "remote-context.h"

namespace yapns {

class UdpSource;
class TrafficAnalyser;
class Packet;

class CallbackVoidPacket {
public:
	CallbackVoidPacket (::Remote::CallbackVoidPacket_ptr remote);
	CallbackVoidPacket (CallbackVoidPacket const &o);
	~CallbackVoidPacket ();
	::Remote::CallbackVoidPacket_ptr peek_remote (void);
private:
	::Remote::CallbackVoidPacket_ptr m_remote;
};

CallbackVoidPacket
make_callback (void (UdpSource::*ptr) (Packet *), UdpSource *self);

CallbackVoidPacket
make_callback (void (TrafficAnalyser::*ptr) (Packet *), TrafficAnalyser *self);

}; // namespace yapns

#endif /* YAPNS_CALLBACK_H */
