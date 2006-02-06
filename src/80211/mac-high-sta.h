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
#ifndef MAC_HIGH_STATION
#define MAC_HIGH_STATION

#include "mac-high.h"
#include "mac-handler.tcc"

class Packet;
class MacParameters;
class MacQueue80211e;
class NetInterface80211;

class MacHighSta : public MacHigh {
public:
	MacHighSta (int apAddress);
	virtual ~MacHighSta ();

	virtual void enqueueFromLL (Packet *packet, int macDestination);

	virtual void receiveFromMacLow (Packet *packet);
	virtual void notifyAckReceivedFor (Packet *packet);

	/* If a subclass of this class needs to send a packet,
	 * it can queue it with this method. This method will
	 * take care of queuing the packet if the station is
	 * not associated and will send it as soon as possible.
	 */
	void queueToLowDirectly (Packet *packet);

protected:
	Packet *getPacketFor (int destination);
	int getApAddress (void);
	void notifySetNetInterface (NetInterface80211 *interface);
private:
	virtual void enqueueToLow (Packet *packet) = 0;
	virtual void gotCFPoll (Packet *packet) = 0;
	virtual void gotBeacon (Packet *packet) = 0;
	virtual void gotAssociated (Packet *packet) = 0;
	virtual void gotReAssociated (Packet *packet) = 0;
	virtual void gotAddTsResponse (Packet *packet) = 0;
	virtual void gotDelTsResponse (Packet *packet) = 0;
	virtual void flush (void) = 0;


	void sendAssociationRequest (void);
	void sendReAssociationRequest (void);
	void sendProbeRequest (void);

	double now (void);
	void tryToEnsureAssociated (void);
	void emptyAssociationQueue (void);
	void recordBeaconRx (Packet *packet);
	bool isBeaconMissed (void);
	bool isAssociated (void);
	void setAssociated (void);
	void setDisAssociated (void);
	MacParameters *parameters (void);

	MacQueue80211e *m_associationQueue;
	NetInterface80211 *m_interface;
	int m_apAddress;
	bool m_associated;
	double m_lastBeaconRx;
	double m_beaconPeriod;
	
};

#endif /* MAC_HIGH_STATION */
