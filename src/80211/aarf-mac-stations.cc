/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2004,2005 INRIA
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

AarfMacStations::AarfMacStations (MacContainer *container)
	: ArfMacStations (container)
{}
AarfMacStations::~AarfMacStations ()
{}
MacStation *
AarfMacStations::createStation (void)
{
	return new AarfMacStation (this, 2.0, 60, 2.0);
}




AarfMacStation::AarfMacStation (MacStations *stations, 	
				double successK,
				int maxSuccessThreshold,
				double timerK)
	: ArfMacStation (stations, 15, 10),
	  m_successK (successK),
	  m_maxSuccessThreshold (maxSuccessThreshold),
	  m_timerK (timerK)
{}


AarfMacStation::~AarfMacStation ()
{}

void 
AarfMacStation::reportRecoveryFailure (void)
{
        setSuccessThreshold ((int)(min (getSuccessThreshold () * m_successK,
					m_maxSuccessThreshold)));
        setTimerTimeout ((int)(max (getMinTimerTimeout (),
                                      getSuccessThreshold () * m_timerK)));
}

void 
AarfMacStation::reportFailure (void)
{
        setTimerTimeout (getMinTimerTimeout ());
        setSuccessThreshold (getMinSuccessThreshold ());
}

