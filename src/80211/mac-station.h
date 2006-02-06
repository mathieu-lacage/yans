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
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
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

	// reception-related method
	virtual void reportRxOk (double SNR, int mode) = 0;

	// transmission-related methods
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
