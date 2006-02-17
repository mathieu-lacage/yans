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
#ifndef MAC_STATIONS_H
#define MAC_STATIONS_H

#include <list>
#include <utility>
#include "mac-address.h"

namespace yans {

class MacStation;

class MacStations {
public:
	MacStations ();
	virtual ~MacStations ();
  
	MacStation *lookup (MacAddress address);

	void set_n_modes (uint32_t n_modes);
	int get_n_modes (void) const;

private:
	typedef std::list <std::pair<MacAddress, MacStation *> > Stations;
	typedef std::list <std::pair<MacAddress, MacStation *> >::iterator StationsI;
	virtual class MacStation *createStation (void) = 0;
	Stations m_stations;
	uint32_t m_n_modes;
};

}; // namespace yans


#endif /* MAC_STATIONS_H */
