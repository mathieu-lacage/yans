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

#ifndef PHY_SNRT_H
#define PHY_SNRT_H

#include "phy-80211.h"
#include "mac-handler.tcc"

class Packet;


class PhySnrt : public Phy80211 {
public:
	PhySnrt ();
	virtual ~PhySnrt ();

private:
	virtual void startRx (Packet *packet);
	virtual void cancelRx (void);
private:
	void endRx (MacCancelableEvent *ev);
	void appendRxEvent (double duration, double power);
	double getCurrentNi (void);
	void removeFinishedEvents (double time);
	double getSnrThreshold (void);
private:
	DynamicHandler<PhySnrt> *m_endRxHandler;
	Packet *m_rxPacket;
	double m_currentNI;
	double m_snrThreshold;
	typedef vector <pair<double, double> >::iterator EndRxEventsI;
	vector <pair<double, double> > m_endRxEvents;
};


#endif /* PHY_SNRT_H */
