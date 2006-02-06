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
#ifndef MAC_HIGH_ADHOC_H
#define MAC_HIGH_ADHOC_H

#include "mac-high.h"

class MacContainer;
class MacQueue80211e;
class Packet;
class Dcf;

class MacHighAdhoc : public MacHigh {
public:
	MacHighAdhoc (MacContainer *container);
	virtual ~MacHighAdhoc ();

	/* invoked by Mac80211. */
	virtual void enqueueFromLL (Packet *packet);

	/* invoked by the MacLows. */
	virtual void notifyAckReceivedFor (Packet *packet);
	virtual void receiveFromMacLow (Packet *packet);
private:
	friend class MyDcfAccessNeedListener;
	bool accessNeeded (void);
	double now (void);
	MacQueue80211e *m_queue;
	Dcf *m_dcf;
};

#endif /* MAC_HIGH_ADHOC_H */
