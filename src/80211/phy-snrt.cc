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

#include "phy-snrt.h"
#include "mac-handler.tcc"
#include "hdr-mac-80211.h"
#include "mac-traces.h"

#include "packet.h"

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
 *       The Phy itself.
 ****************************************************************/

PhySnrt::PhySnrt ()
	: Phy80211 ()
{
	m_endRxHandler = new DynamicHandler<PhySnrt> (this, &PhySnrt::endRx);
	m_rxPacket = 0;
	m_snrThreshold = 1;
	m_currentNI = 0.0;
}

PhySnrt::~PhySnrt ()
{
	delete m_endRxHandler;
}

double
PhySnrt::getSnrThreshold (void)
{
	return m_snrThreshold;
}

void
PhySnrt::removeFinishedEvents (double time)
{
	EndRxEventsI i = m_endRxEvents.begin ();
	while (i != m_endRxEvents.end ()) {
		if ((*i).first < time) {
			m_currentNI -= (*i).second;
			i = m_endRxEvents.erase (i);
		} else {
			i++;
		}
	}
}

void
PhySnrt::appendRxEvent (double duration, double power)
{
	removeFinishedEvents (now ());
	m_endRxEvents.push_back (make_pair (now ()+duration, power));
	m_currentNI += power;
}

double
PhySnrt::getCurrentNi (void)
{
	removeFinishedEvents (now ());
	return m_currentNI;
}

void 
PhySnrt::startRx (Packet *packet)
{
	double power = calculateRxPower (packet);
	double rxDuration = calculatePacketDuration (0, getPayloadMode (packet),
						     ::getSize (packet));
	appendRxEvent (rxDuration, power);
	switch (getState ()) {
	case Phy80211::SYNC:
		TRACE ("drop packet because already in sync (power: %e until %f)", power, now () + rxDuration);
		assert (m_rxPacket != 0);
		Packet::free (packet);
		break;
	case Phy80211::TX:
		TRACE ("drop packet because already in tx (power: %e until %f)", power, now () + rxDuration);
		Packet::free (packet);
		break;
	case Phy80211::SLEEP:
		Packet::free (packet);
		break;
	case Phy80211::IDLE: {
		assert (m_rxPacket == 0);
		
		if (power > dBmToW (getRxThreshold ())) {
			// sync to signal
			TRACE ("sync on packet (power: %e until %f)", power, now () + rxDuration);
			notifyRxStart (now (), rxDuration);
			switchToSyncFromIdle (rxDuration);
			m_rxPacket = packet;
			m_endRxHandler->start (rxDuration);
		} else {
			/* if the energy of the signal is smaller than rxThreshold,
			 * this packet is not synced upon.
			 */
			TRACE ("drop packet (power: %e until %f)", power, now () + rxDuration);
			Packet::free (packet);
		}
	} break;
	}
}

void
PhySnrt::endRx (MacCancelableEvent *ev)
{
	assert (getState () == Phy80211::SYNC);

	double power = calculateRxPower (m_rxPacket);
	double ni = getCurrentNi ();
	double snr = SNR (power, 
			  ni - power, 
			  getMode (getPayloadMode (m_rxPacket)));
	TRACE ("end rx (snr=%e/%e=%f)", power, ni - power, snr)
	setLastRxSNR (snr);

	bool receivedOk;
	if (snr > getSnrThreshold ()) {
		receivedOk = true;
	} else {
		receivedOk = false;
	}
	if (receivedOk) {
		HDR_CMN (m_rxPacket)->error () = 0;
	} else {
		HDR_CMN (m_rxPacket)->error () = 1;
	}
	HDR_CMN (m_rxPacket)->direction() = hdr_cmn::UP;
	
	notifyRxEnd (now (), receivedOk);
	switchToIdleFromSync ();
	forwardUp (m_rxPacket);
	m_rxPacket = 0;
}

void 
PhySnrt::cancelRx (void)
{
	m_endRxHandler->cancel ();
	Packet::free (m_rxPacket);
	m_rxPacket = 0;
}

