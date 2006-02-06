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

#ifndef NET_INTERFACE_H
#define NET_INTERFACE_H

#include <tclcl.h>

class NodeEmpty;
class BroadcastChannel;
class Packet;

class NetInterface {
public:
	NetInterface ();
	virtual ~NetInterface ();

	void connectTo (BroadcastChannel *channel, NodeEmpty *node);

	virtual void sendDown (Packet *packet) = 0;
	virtual void sendUp (Packet *packet) = 0;

protected:
	void sendDownToChannel (Packet *packet);
	void sendUpToNode (Packet *packet);
private:
	NodeEmpty *m_node;
	BroadcastChannel *m_channel;
};


class NetInterfaceConstructor : public TclObject {
public:
	NetInterfaceConstructor ();
	virtual ~NetInterfaceConstructor ();

	virtual NetInterface *createInterface (void) = 0;
};

#endif /* NET_INTERFACE_H */
