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

#include "mac-station.h"
#include "mac-stations.h"

MacStation::MacStation (MacStations *stations)
	: m_sequence (0),
	  m_stations (stations)
{}


MacStation::~MacStation ()
{}


void
MacStation::setLastRxSequence (int sequence)
{
	m_sequence = sequence;
}
int
MacStation::getLastRxSequence (void)
{
	return m_sequence;
}

int
MacStation::getNModes (void)
{
	return m_stations->getNModes ();
}
bool 
MacStation::isAssociated (void)
{
	return m_associated;
}
void 
MacStation::recordAssociated (void)
{
	m_associated = true;
}
