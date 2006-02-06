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

#ifndef SIMPLE_NET_INTERFACE_H
#define SIMPLE_NET_INTERFACE_H

#include <tclcl.h>

#include "net-interface.h"

class Delay;

class SimpleNetInterface : public NetInterface {
public:
	SimpleNetInterface (double delay);
	virtual ~SimpleNetInterface ();

	virtual void sendDown (Packet *packet);
	virtual void sendUp (Packet *packet);

private:
	friend class Delay;
	void reallySendDown (Packet *packet);
	Delay *m_delay;
};

class SimpleNetInterfaceConstructor : public NetInterfaceConstructor {
public:
	SimpleNetInterfaceConstructor ();
	virtual ~SimpleNetInterfaceConstructor ();

	virtual NetInterface *createInterface (void);
};

#endif /* NET_INTERFACE_H */
