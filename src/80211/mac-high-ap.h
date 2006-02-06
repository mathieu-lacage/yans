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
#ifndef MAC_HIGH_AP
#define MAC_HIGH_AP

#include "mac-high.h"

class MacContainer;
class Packet;
class MacDcfParameters;
class MacQueue80211e;
class Dcf;
class MacParameters;

class MacHighAp : public MacHigh {
public:
	MacHighAp (MacContainer *container);
	virtual ~MacHighAp ();

	virtual void enqueueFromLL (Packet *packet);
	virtual void receiveFromMacLow (Packet *packet);
	virtual void notifyAckReceivedFor (Packet *packet);
 private:
	virtual void sendAssociationResponseOk (int destination) = 0;
	virtual void sendReAssociationResponseOk (int destination) = 0;
	virtual void sendProbeResponse (int destination) = 0;
	virtual void enqueueToLow (Packet *packet) = 0;
	virtual void forwardQueueToLow (Packet *packet) = 0;
	virtual void gotAddTsRequest (Packet *packet) = 0;
	virtual void gotDelTsRequest (Packet *packet) = 0;
	virtual void gotCFPoll (Packet *packet) = 0;
	virtual void gotQosNull (Packet *packet) = 0;
};

#endif /* MAC_HIGH_AP */
