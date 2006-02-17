/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2004,2005,2006 INRIA
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

#include "aarf-mac-stations.h"

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

namespace yans {

AarfMacStations::AarfMacStations ()
	: ArfMacStations ()
{}
AarfMacStations::~AarfMacStations ()
{}
MacStation *
AarfMacStations::createStation (void)
{
	return new AarfMacStation (this, 2.0, 60, 2.0);
}




AarfMacStation::AarfMacStation (MacStations *stations, 	
				double success_k,
				int max_success_threshold,
				double timer_k)
	: ArfMacStation (stations, 15, 10),
	  m_success_k (success_k),
	  m_max_success_threshold (max_success_threshold),
	  m_timer_k (timer_k)
{}


AarfMacStation::~AarfMacStation ()
{}

void 
AarfMacStation::report_recovery_failure (void)
{
        set_success_threshold ((int)(min (get_success_threshold () * m_success_k,
					m_max_success_threshold)));
        set_timer_timeout ((int)(max (get_min_timer_timeout (),
                                      get_success_threshold () * m_timer_k)));
}

void 
AarfMacStation::report_failure (void)
{
        set_timer_timeout (get_min_timer_timeout ());
        set_success_threshold (get_min_success_threshold ());
}

}; // namespace yans
