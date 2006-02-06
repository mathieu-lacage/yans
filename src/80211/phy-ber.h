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

#ifndef PHY_BER_H
#define PHY_BER_H

#include <list>

#include "phy-80211.h"

class Packet;
class PhyRxEvent;
class NIChange;
class EndRxHandler;
class RngUniform;

class PhyBer : public Phy80211 {
public:
	PhyBer ();
	virtual ~PhyBer ();

private:
	virtual void startRx (Packet *packet);
	virtual void cancelRx (void);
private:
	class NIChange {
	public:
		NIChange (double time, double delta);
		double getTime (void);
		double getDelta (void);
		bool operator < (NIChange a) const;
	private:
		double m_time;
		double m_delta;
	};

	double calculateCurrentNoiseInterference (void);
	double calculateNoiseInterference (double time);
	double calculateNI (PhyRxEvent *phyRxEvent, vector <NIChange> *snir);
	double calculatePER (PhyRxEvent *packet, vector <NIChange> *snir);
	double calculateChunkSuccessRate (double snir, double delay, TransmissionMode *mode);
	void appendEvent (PhyRxEvent *event);
	void endRx (PhyRxEvent *phyRxEvent, Packet *packet);
private:
	list<PhyRxEvent *> m_rxEventList;
	EndRxHandler *m_endRxHandler;
	static RngUniform *m_random;

	friend class PhyRxEvent;
};

#endif /* PHY_BER_H */
