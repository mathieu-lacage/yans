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

#ifndef PHY_ET_H
#define PHY_ET_H

#include "phy-80211.h"
#include "mac-handler.tcc"

class Packet;


class PhyEt : public Phy80211 {
public:
	PhyEt ();
	virtual ~PhyEt ();

private:
	virtual void startRx (Packet *packet);
	virtual void cancelRx (void);
private:
	void endRx (MacCancelableEvent *ev);
private:
	DynamicHandler<PhyEt> *m_endRxHandler;
	Packet *m_rxPacket;
};


#endif /* PHY_ET_H */
