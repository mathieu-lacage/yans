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

#include "rate-control.h"
#include "sta-rate-control.h"
#include "sta-rate-control-factory.h"


RateControl::RateControl (class StaRateControlFactory *factory)
	: m_factory (factory),
	  m_broadcast (0)
{}

RateControl::~RateControl ()
{
	delete m_factory;
}

class StaRateControl *
RateControl::peekRateControl (int destination)
{
	if (m_destinations[destination] == 0) {
		m_destinations[destination] = m_factory->createStaRateControl ();
	}
	return m_destinations[destination];
}

void 
RateControl::reportRxOk (int source, double SNR, int mode)
{
	peekRateControl (source)->reportRxOk (SNR, mode);
}

void 
RateControl::reportRTSFailed (int destination)
{
	peekRateControl (destination)->reportRTSFailed ();
}
void 
RateControl::reportDataFailed (int destination)
{
	peekRateControl (destination)->reportDataFailed ();
}
void 
RateControl::reportRTSOk (int destination, double ctsSNR, int ctsMode)
{
	peekRateControl (destination)->reportRTSOk (ctsSNR, ctsMode);
}
void 
RateControl::reportDataOk (int destination, double ackSNR, int ackMode)
{
	peekRateControl (destination)->reportDataOk (ackSNR, ackMode);
}

void 
RateControl::reportFinalRTSFailed (int destination)
{
	peekRateControl (destination)->reportFinalRTSFailed ();
}

void 
RateControl::reportFinalDataFailed (int destination)
{
	peekRateControl (destination)->reportFinalDataFailed ();
}

int 
RateControl::getDataMode (int destination, int size)
{
	int mode;
	mode = peekRateControl (destination)->getDataMode (size);
	return mode;
}

int 
RateControl::getRTSMode (int destination)
{
	int mode;
	mode = peekRateControl (destination)->getRTSMode ();
	return mode;
}

int 
RateControl::getBroadcastDataMode (int size)
{
	if (m_broadcast == 0) {
		m_broadcast = m_factory->createBroadcastStaRateControl ();
	}
	int mode = m_broadcast->getDataMode (size);
	return mode;
}
