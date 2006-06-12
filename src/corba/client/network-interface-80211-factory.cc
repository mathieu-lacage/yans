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
#include "network-interface-80211-factory.h"
#include "network-interface-80211.h"
#include "id-factory.h"
#include "position.h"

namespace yapns {

NetworkInterface80211Factory::NetworkInterface80211Factory ()
{}
NetworkInterface80211Adhoc *
NetworkInterface80211Factory::create_adhoc (SimulationContext ctx, 
					    MacAddress address, 
					    Position *position)
{
	struct ::Remote::MacAddress corba_address;
	address.peek (corba_address.data);
	::Remote::NetworkInterface80211Factory_ptr remote_factory = 
		  ctx->peek_80211_factory ();
	::Remote::PositionModel_ptr remote_position = position->get_remote ();
	// XXX here, set _all_ the parameters on the factory.
	::Remote::NetworkInterface80211Adhoc_ptr remote = 
		  remote_factory->create_adhoc (corba_address, 
						remote_position,
						IdFactory::get_next ());
	NetworkInterface80211Adhoc *local = new NetworkInterface80211Adhoc (remote);
	CORBA::release (remote);
	CORBA::release (remote_position);
	return local;
}


}; // namespace yapns
