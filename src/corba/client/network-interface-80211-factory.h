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
#ifndef YAPNS_NETWORK_INTERFACE_80211_FACTORY_H
#define YAPNS_NETWORK_INTERFACE_80211_FACTORY_H

#include "mac-address.h"
#include "simulation-context.h"

namespace yapns {

class Position;
class NetworkInterface80211Adhoc;

class NetworkInterface80211Factory {
public:
	NetworkInterface80211Factory ();
	NetworkInterface80211Adhoc *create_adhoc (SimulationContext ctx, 
						  MacAddress address, 
						  Position *position);
};

}; // namespace yapns

#endif /* YAPNS_NETWORK_INTERFACE_80211_FACTORY_H */
