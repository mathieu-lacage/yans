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
#ifndef MAC_PARAMETERS_H
#define MAC_PARAMETERS_H

class Phy80211;
class Mac80211;

class MacParameters {
public:
	MacParameters (Mac80211 *mac, Phy80211 *phy);
	
	int getACKSize (void) const;
	int getRTSSize (void) const;
	int getCTSSize (void) const;

	double getBeaconInterval (void);
	int getMaxMissedBeacon (void);
	int getBeaconSize (void);
	int getAssociationResponseSize (void);
	int getReAssociationResponseSize (void);
	int getProbeResponseSize (void);
	int getAssociationRequestSize (void);
	int getReAssociationRequestSize (void);
	int getProbeRequestSize (void);


	int getDataHeaderSize (void);
	int getMgtHeaderSize (void);

	double getSIFS (void);
	double getSlotTime (void);
	int getCWmin (void);
	int getCWmax (void);

	int getMaxSSRC (void);
	int getMaxSLRC (void);
	int getRTSCTSThreshold (void);
	int getFragmentationThreshold (void);
	double getCTSTimeoutDuration (void);
	double getACKTimeoutDuration (void);

	double getEIFS (void);
	double getDIFS (void);
private:
	int getSelf (void);
	Phy80211 *peekPhy (void);
	Mac80211 *m_mac;
	Phy80211 *m_phy;
};

#endif /* MAC_PARAMETERS_H */
