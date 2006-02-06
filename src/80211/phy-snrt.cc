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

#include "phy-snrt.h"
#include "mac-handler.tcc"
#include "hdr-mac-80211.h"
#include "mac-traces.h"

#ifndef PHY_SNRT_TRACE
#define nopePHY_SNRT_TRACE 1
#endif /* PHY_SNRT_TRACE */

#ifdef PHY_SNRT_TRACE
# define TRACE(format, ...) \
	printf ("SNRT TRACE %d %f " format "\n", selfAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* PHY_SNRT_TRACE */
# define TRACE(format, ...)
#endif /* PHY_SNRT_TRACE */


/****************************************************************
 *       TCL glue.
 ****************************************************************/

static class PhySnrtClass: public TclClass {
public:
        PhySnrtClass() : TclClass("Phy/80211/Snrt") {}
        TclObject* create(int, const char*const*) {
                return (new PhySnrt ());
        }
} class_PhySnrt;

/****************************************************************
 *       The Phy itself.
 ****************************************************************/

PhySnrt::PhySnrt ()
	: Phy80211 ()
{
	m_endRxHandler = new DynamicHandler<PhySnrt> (this, &PhySnrt::endRx);
	m_rxPacket = 0;
}

PhySnrt::~PhySnrt ()
{
	delete m_endRxHandler;
}

void 
PhySnrt::startRx (Packet *packet)
{
	switch (getState ()) {
	case Phy80211::SYNC:
		TRACE ("drop packet because already in sync");
		assert (m_rxPacket != 0);
		Packet::free (packet);
		break;
	case Phy80211::TX:
		TRACE ("drop packet because already in tx");
		Packet::free (packet);
		break;
	case Phy80211::SLEEP:
		Packet::free (packet);
		break;
	case Phy80211::IDLE: {
		assert (m_rxPacket == 0);
		double power = calculatePower (packet);
		
		if (power > dBmToW (getRxThreshold ())) {
			// sync to signal
			TRACE ("sync on packet");
			double rxDuration = calculatePacketDuration (0, getPayloadMode (packet),
								     ::getSize (packet));
			notifyRxStart (now (), rxDuration);
			switchToSyncFromIdle (rxDuration);
			m_rxPacket = packet;
			m_endRxHandler->start (rxDuration);
		} else {
			/* if the energy of the signal is smaller than rxThreshold,
			 * this packet is not synced upon.
			 */
			TRACE ("drop packet");
			Packet::free (packet);
		}
	} break;
	}
}

void
PhySnrt::endRx (MacCancelableEvent *ev)
{
	assert (getState () == Phy80211::SYNC);

	// XXX ???
	setLastRxSNR (calculatePower (m_rxPacket));

	HDR_CMN (m_rxPacket)->error () = 0;
	HDR_CMN (m_rxPacket)->direction() = hdr_cmn::UP;
	
	bool receivedOk = true;
	notifyRxEnd (now (), receivedOk);
	switchToIdleFromSync ();
	uptarget ()->recv (m_rxPacket);
	m_rxPacket = 0;
}

void 
PhySnrt::cancelRx (void)
{
	m_endRxHandler->cancel ();
	Packet::free (m_rxPacket);
	m_rxPacket = 0;
}

