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

#ifndef PHY_BER_H
#define PHY_BER_H

#include <list>
#include <vector>

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
	typedef std::vector <NIChange> NIChanges;
	typedef std::vector <NIChange>::iterator NIChangesI;

	double calculateCurrentNoiseInterference (void);
	double calculateNoiseInterference (double time);
	double calculateNI (PhyRxEvent *phyRxEvent, NIChanges *snir);
	double calculatePER (PhyRxEvent *packet, NIChanges *snir);
	double calculateChunkSuccessRate (double snir, double delay, TransmissionMode *mode);
	void appendEvent (PhyRxEvent *event);
	void endRx (PhyRxEvent *phyRxEvent, Packet *packet);
private:
	typedef std::list<PhyRxEvent *> PhyRxEvents;
	typedef std::list<PhyRxEvent *>::iterator PhyRxEventsI;
	PhyRxEvents m_rxEventList;
	EndRxHandler *m_endRxHandler;
	static RngUniform *m_random;

	friend class PhyRxEvent;
};

#endif /* PHY_BER_H */
