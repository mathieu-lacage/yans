/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#ifndef ETHERNET_NETWORK_INTERFACE_H
#define ETHERNET_NETWORK_INTERFACE_H

#include "yans/mac-network-interface.h"
#include "yans/mac-address.h"
#include "yans/packet.h"

namespace yans {

class Cable;
class TraceContainer;
class PacketLogger;

class EthernetNetworkInterface : public MacNetworkInterface {
public:
	EthernetNetworkInterface (MacAddress address, 
				  char const *name);
	virtual ~EthernetNetworkInterface ();


	void connect_to (Cable *cable);
	void recv (Packet packet);

	void register_trace (TraceContainer *container);

 private:
	virtual void notify_up (void);
	virtual void notify_down (void);
	virtual void real_send (Packet packet, MacAddress to);

	Cable *m_cable;
	PacketLogger *m_send_logger;
	PacketLogger *m_recv_logger;
};

}; // namespace yans

#endif /* ETHERNET_NETWORK_INTERFACE_H */
