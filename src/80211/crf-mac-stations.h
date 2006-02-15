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
#ifndef CRF_MAC_STATIONS_H
#define CRF_MAC_STATIONS_H

#include "mac-stations.h"
#include "mac-station.h"

class MacContainer;

class CrfMacStations : public MacStations {
public:
	CrfMacStations (MacContainer *container,
			int dataRate, int ctlRate);
	virtual ~CrfMacStations ();
private:
	virtual class MacStation *createStation (void);

	int m_dataRate;
	int m_ctlRate;
};


class CrfMacStation : public MacStation
{
public:
	CrfMacStation (MacStations *stations, int dataRate, int ctlRate);
	virtual ~CrfMacStation ();

	virtual void reportRxOk (double SNR, int mode);

	virtual void reportRTSFailed (void);
	virtual void reportDataFailed (void);
	virtual void reportRTSOk (double ctsSNR, int ctsMode);
	virtual void reportDataOk (double ackSNR, int ackMode);
	virtual void reportFinalRTSFailed (void);
	virtual void reportFinalDataFailed (void);
	virtual int getDataMode (int size);
	virtual int getRTSMode (void);

private:
	int m_dataRate;
	int m_ctlRate;
};



#endif /* CRF_MAC_STATIONS_H */