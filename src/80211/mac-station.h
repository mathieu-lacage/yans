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
#ifndef MAC_STATION_H
#define MAC_STATION_H

class MacStations;

class MacStation {
public:
	MacStation (MacStations *stations);

	virtual ~MacStation ();

	/* All stations are assumed to be 
	 * Authenticated so this method always
	 * returns true.
	 */
	bool isAssociated (void);
	void recordAssociated (void);

	void setLastRxSequence (int sequence);
	int getLastRxSequence (void);

	virtual void reportRxOk (double SNR, int mode) = 0;
	virtual void reportRTSFailed (void) = 0;
	virtual void reportDataFailed (void) = 0;
	virtual void reportRTSOk (double ctsSNR, int ctsMode) = 0;
	virtual void reportDataOk (double ackSNR, int ackMode) = 0;
	virtual void reportFinalRTSFailed (void) = 0;
	virtual void reportFinalDataFailed (void) = 0;
	virtual int getDataMode (int size) = 0;
	virtual int getRTSMode (void) = 0;

protected:
	int getNModes (void);
private:
	int m_sequence;
	bool m_associated;
	MacStations *m_stations;
};

#endif /* MAC_STA_H */
