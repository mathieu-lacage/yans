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
#ifndef MAC_HIGH_H
#define MAC_HIGH_H

class Mac80211;
class Phy80211;
class Packet;
class MacStations;
class MacStation;
class MacParameters;

class MacHigh {
public:
	MacHigh (Mac80211 *mac, Phy80211 *phy);
	virtual ~MacHigh ();

	/* invoked by Mac80211. */
	virtual void enqueueFromLL (Packet *packet) = 0;
	virtual void receiveFromPhy (Packet *packet) = 0;

	/* invoked by the MacLows. */
	MacStation *lookupStation (int address);
	virtual void notifyAckReceivedFor (Packet *packet) = 0;
	virtual void receiveFromMacLow (Packet *packet) = 0;
protected:
	Packet *getPacketFor (int addresss);
	int getSelf (void);
	void forwardUp (Packet *packet);
	MacParameters *parameters (void);
	Mac80211 *peekMac80211 (void);
	Phy80211 *peekPhy80211 (void);
private:
	Mac80211 *m_mac;
	Phy80211 *m_phy;
	MacStations *m_stations;
};

#endif /* MAC_HIGH_H */
