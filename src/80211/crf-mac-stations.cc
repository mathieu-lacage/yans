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

#include "mac-stations.h"
#include "mac-station.h"
#include "crf-mac-stations.h"

#include <assert.h>

CrfMacStation::CrfMacStation (MacStations *stations, int dataRate, int ctlRate)
	: MacStation (stations),
	  m_dataRate (dataRate),
	  m_ctlRate (ctlRate)
{}
CrfMacStation::~CrfMacStation ()
{}

void 
CrfMacStation::reportRTSFailed (void)
{}
void 
CrfMacStation::reportDataFailed (void)
{}
void 
CrfMacStation::reportRxOk (double SNR, int mode)
{}
void CrfMacStation::reportRTSOk (double ctsSNR, int ctsMode)
{}
void CrfMacStation::reportDataOk (double ackSNR, int ackMode)
{}
void CrfMacStation::reportFinalRTSFailed (void)
{}
void CrfMacStation::reportFinalDataFailed (void)
{}
int CrfMacStation::getDataMode (int size)
{
	return m_dataRate;
}
int CrfMacStation::getRTSMode (void)
{
	return m_ctlRate;
}






CrfMacStations::CrfMacStations (MacContainer *container, int dataRate, int ctlRate)
	: MacStations (container),
	  m_dataRate (dataRate),
	  m_ctlRate (ctlRate)
{}
CrfMacStations::~CrfMacStations ()
{}
MacStation *
CrfMacStations::createStation (void)
{
	/* XXX: use mac to access user and PHY params. */
	return new CrfMacStation (this, m_dataRate, m_ctlRate);
}

