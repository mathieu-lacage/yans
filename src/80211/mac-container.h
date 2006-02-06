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


#ifndef MAC_CONTAINER_H
#define MAC_CONTAINER_H

class MacStations;
class MacParameters;
class MacLow;
class MacRxMiddle;
class MacTxMiddle;
class MacHigh;
class Phy80211;
class Mac80211;
class Packet;

class MacContainer 
{
 public:
	MacContainer (Mac80211 *nsMac);

	MacParameters *parameters (void);
	MacLow *macLow (void);
	MacRxMiddle *macRxMiddle (void);
	MacTxMiddle *macTxMiddle (void);
	MacHigh *macHigh (void);
	Phy80211 *phy (void);
	MacStations *stations (void);

	int selfAddress (void);
	void forwardToPhy (Packet *packet);
	void forwardToLL (Packet *packet);

	/* this is the order in which these methods
	 * must be invoked for proper intialization.
	 */
	void setPhy (Phy80211 *phy);
	void setStations (MacStations *stations);
	void setParameters (MacParameters *parameters);
	void setMacLow (MacLow *low);
	void setMacRxMiddle (MacRxMiddle *middle);
	void setMacTxMiddle (MacTxMiddle *middle);
	void setMacHigh (MacHigh *high);
 private:
	Mac80211 *nsMac (void);
	MacParameters *m_parameters;
	MacLow *m_low;
	MacRxMiddle *m_middle;
	MacTxMiddle *m_txMiddle;
	MacHigh *m_high;
	Phy80211 *m_phy;
	Mac80211 *m_nsMac;
	MacStations *m_stations;
};

#endif /* MAC_CONTAINER_H */
