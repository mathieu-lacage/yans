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

#include "mac-stations.h"

namespace yans {

MacStations::MacStations ()
{}

MacStations::~MacStations ()
{}

MacStation *
MacStations::lookup (MacAddress address)
{
	for (StationsI i = m_stations.begin (); i != m_stations.end (); i++) {
		if ((*i).first.is_equal (address)) {
			return (*i).second;
		}
	}
	MacStation *station = create_station ();
	m_stations.push_back (std::make_pair (address, station));
	return station;
}

}; // namespace yans
