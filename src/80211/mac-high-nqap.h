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
#ifndef MAC_HIGH_NQAP
#define MAC_HIGH_NQAP

#include "mac-high-ap.h"
#include "mac-handler.tcc"

class MacContainer;
class Packet;
class MacDcfParameters;
class MacQueue80211e;
class Dcf;
class MacParameters;

class MacHighNqap : public MacHighAp {
public:
	MacHighNqap (MacContainer *container);
	virtual ~MacHighNqap ();

	virtual void addTsRequest (TSpecRequest *request);
	virtual void delTsRequest (TSpecRequest *request);

private:
	MacParameters *parameters (void);
	Packet *getPacketFor (int destination);
	void sendBeacon (void);

	virtual void enqueueToLow (Packet *packet);
	virtual void forwardQueueToLow (Packet *packet);
	virtual void sendAssociationResponseOk (int destination);
	virtual void sendReAssociationResponseOk (int destination);
	virtual void sendProbeResponse (int destination);
	virtual void gotAddTsRequest (Packet *packet);
	virtual void gotDelTsRequest (Packet *packet);
	virtual void gotCFPoll (Packet *packet);

	MacDcfParameters *m_dcfParameters;
	MacQueue80211e *m_queue;
	Dcf *m_dcf;

	StaticHandler<MacHighNqap> *m_sendBeaconTimer;
};

#endif /* MAC_HIGH_NQAP */
