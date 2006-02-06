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

#include "phy-80211.h"
#include "bpsk-mode.h"
#include "qam-mode.h"
#include "rng-uniform.h"
#include "hdr-mac-80211.h"
#include "net-interface.h"
#include "free-space-propagation.h"
#include "transmission-mode.h"
#include "mac-low.h"

#include "packet.h"

#include <math.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cfloat>

#define nopePHY80211_DEBUG 1
#define nopePHY80211_STATE_DEBUG 1

/* J/K */
#define BOLTZMANN 1.3803e-23
/* W = J/s */

/* All the state transitions are marked by these macros. */
#ifdef PHY80211_STATE_DEBUG
#  define STATE_FROM(from) \
     printf ("PHY TRACE %d from %s", this->node_->address (), stateToString (from));
#  define STATE_TO(to) \
     printf (" to %s", stateToString (to));
#  define STATE_AT(at) \
     printf (" at %g\n", at);
#else
#  define STATE_FROM(from)
#  define STATE_TO(from)
#  define STATE_AT(at)
#endif

/****************************************************************
 *       This destructor is needed.
 ****************************************************************/

Phy80211Listener::~Phy80211Listener ()
{}

/****************************************************************
 *       The actual Phy80211 class
 ****************************************************************/

Phy80211::Phy80211 ()
	: m_sleeping (false),
	  m_rxing (false),
	  m_endTx (0.0),
	  m_previousStateChangeTime (0.0)
{}

Phy80211::~Phy80211 ()
{}

void
Phy80211::setInterface (NetInterface *interface)
{
	m_interface = interface;
}
void 
Phy80211::setMac (MacLow *low)
{
	m_mac = low;
}

void 
Phy80211::forwardUp (Packet *packet)
{
	m_mac->receive (packet);
}

void 
Phy80211::setRxThreshold (double rxThreshold)
{
	m_rxThreshold = rxThreshold;
}
void 
Phy80211::setRxNoise (double rxNoise)
{
	m_rxNoise = rxNoise;
}
void 
Phy80211::setTxPower (double txPower)
{
	m_txPower = txPower;
}

void
Phy80211::configureStandardA (void)
{
	m_plcpHeaderLength = 4 + 1 + 12 + 1 + 6 + 16 + 6;
	m_plcpPreambleDelay = 20e-6;
	/* 4095 bytes at a 6Mb/s rate with a 1/2 coding rate. */
	m_maxPacketDuration = 4095.0*8.0/6000000.0*(1.0/2.0);
	addTxRxMode (new BPSKFECMode (10, 11,       20e6, 6e6, 0.5));
	addTxRxMode (new BPSKFECMode (5, 8,         20e6, 9e6, 0.75));
	addTxRxMode (new QAMFECMode (4, 10, 11, 0,  20e6, 12e6, 0.5));
	addTxRxMode (new QAMFECMode (4, 5, 8, 31,   20e6, 18e6, 0.75));
	addTxRxMode (new QAMFECMode (16, 10, 11, 0, 20e6, 24e6, 0.5));
	addTxRxMode (new QAMFECMode (16, 5, 8, 31,  20e6, 36e6, 0.75));
	addTxRxMode (new QAMFECMode (64, 6, 1, 16,  20e6, 48e6, 0.666));
	addTxRxMode (new QAMFECMode (64, 5, 8, 31,  20e6, 54e6, 0.75));
}

void 
Phy80211::setPropagationModel (FreeSpacePropagation *propagation)
{
	m_propagation = propagation;
}

void 
Phy80211::registerListener (Phy80211Listener *listener)
{
	m_listeners.push_back (listener);
}

void 
Phy80211::notifyRxStart (double now, double duration)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyRxStart (now, duration);
	}
}
void 
Phy80211::notifyRxEnd (double now, bool receivedOk)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyRxEnd (now, receivedOk);
	}
}
void 
Phy80211::notifyTxStart (double now, double duration)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyTxStart (now, duration);
	}
}
void 
Phy80211::notifySleep (double now)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifySleep (now);
	}
}
void 
Phy80211::notifyWakeup (double now)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyWakeup (now);
	}
}


char const *
Phy80211::stateToString (enum Phy80211State state)
{
	switch (state) {
	case Phy80211::SYNC:
		return "SYNC";
		break;
	case Phy80211::TX:
		return "TX";
		break;
	case Phy80211::IDLE:
		return "IDLE";
		break;
	case Phy80211::SLEEP:
		return "SLEEP";
		break;
	}
	return "deadbeaf";
}

int
Phy80211::selfAddress (void)
{
	return m_interface->getMacAddress ();
}

double 
Phy80211::dBmToW (double dBm)
{
	double mW = pow(10.0,dBm/10.0);
	return mW / 1000.0;
}
double 
Phy80211::dBToRatio (double dB)
{
	double ratio = pow(10.0,dB/10.0);
	return ratio;
}
double 
Phy80211::getLastRxSNR (void)
{
	return m_rxStartSNR;
}
void
Phy80211::setLastRxSNR (double snr)
{
	m_rxStartSNR = snr;
}
double 
Phy80211::getMaxPacketDuration (void)
{
	return m_maxPacketDuration;
}
int 
Phy80211::getNModes (void)
{
	return m_modes.size ();
}
double
Phy80211::now (void)
{
	double now;
	now = Scheduler::instance ().clock ();
	return now;
}
double
Phy80211::getRxThreshold (void)
{
	return m_rxThreshold;
}
int
Phy80211::getHeaderMode (Packet *packet)
{
	return 0;
}
int
Phy80211::getPayloadMode (Packet *packet)
{
	return HDR_MAC_80211 (packet)->getTxMode ();
}
double 
Phy80211::getPreambleDuration (void)
{
	return m_plcpPreambleDelay;
}
double 
Phy80211::getEndOfTx (void)
{
	return m_endTx;
}
double 
Phy80211::getEndOfRx (void)
{
	return m_endRx;
}
double
Phy80211::max (double a, double b)
{
	if (a > b) {
		return a;
	} else {
		return b;
	}
}

void
Phy80211::addTxRxMode (TransmissionMode *mode)
{
	m_modes.push_back (mode);
}

void 
Phy80211::sleep (void)
{
	/* sleeping should never be tried if we are transmitting data
	 * because it is not possible to stop the reception of
	 * the corresponding packet for the nodes connected to this
	 * wireless medium. It would result in incorrect simulations.
	 */
	assert (getState () != Phy80211::TX);
	if (getState () == Phy80211::SYNC) {
		cancelRx ();
	}
	notifySleep (now ());
	switchToSleep ();
}

void
Phy80211::wakeup (void)
{
	/* Obviously, wakeup should happen only if we
	 * went to sleep earlier.
	 */
	assert (getState () == Phy80211::SLEEP);
	notifyWakeup (now ());
	switchToIdleFromSleep ();
}




double
Phy80211::calculateHeaderDuration (int headerMode)
{
	return m_plcpHeaderLength / getMode (headerMode)->getDataRate ();
}

double
Phy80211::calculatePacketDuration (int headerMode, int payloadMode, int size)
{
	double delay = getPreambleDuration ();
	delay += calculateHeaderDuration (headerMode);
	delay += (size * 8) / getMode (payloadMode)->getDataRate ();
	return delay;
}

double 
Phy80211::calculateTxDuration (int payloadMode, int size)
{
	return calculatePacketDuration (0, payloadMode, size);
}

/* signalSpread: Hz or 1/s
 * returns Watts
 */
double
Phy80211::calculateNoiseFloor (double signalSpread)
{
	// thermal noise at 290K in J/s = W
	double Nt = BOLTZMANN * 290.0 * signalSpread;
	double Nr = dBToRatio (m_rxNoise);
	// receiver noise floor (W)
	return Nr * Nt;
}

double
Phy80211::SNR (double signal, double noiseInterference, TransmissionMode *mode)
{
	double noise = calculateNoiseFloor (mode->getSignalSpread ()) + noiseInterference;
	double snr = signal / noise;
	return snr;
}


/* return power in W */
double 
Phy80211::calculateRxPower (Packet *p)
{
	return m_propagation->getReceptionPower (p);
}


void
Phy80211::sendUp (Packet *packet)
{
	startRx (packet);
}
void
Phy80211::sendDown (Packet *packet)
{
	/* Transmission can happen if:
	 *  - we are syncing on a packet. It is the responsability of the
	 *    MAC layer to avoid doing this but the PHY does nothing to 
	 *    prevent it.
	 *  - we are idle
	 */
	assert (getState () == Phy80211::IDLE ||
		getState () == Phy80211::SYNC);

	if (getState () == Phy80211::SYNC) {
		cancelRx ();
	}

	/* txPower should be calculated on a per-packet basis by the MAC.
	 * It is actually a matter of:
	 *    - getting the MAC to set this field.
	 *    - leave this field alone here.
	 * XXX
	 */
	m_propagation->setTransmissionPower (packet, m_txPower);
	double txDuration = calculatePacketDuration (getHeaderMode (packet), 
						     getPayloadMode (packet),
						     getSize (packet));
	notifyTxStart (now (), txDuration);
	switchToTx (txDuration);
	m_interface->sendDownToChannel (packet);
}



TransmissionMode *
Phy80211::getMode (int mode)
{
	return m_modes[mode];
}


void
Phy80211::switchToTx (double txDuration)
{
	assert (m_endTx == 0);
	switch (getState ()) {
	case Phy80211::SYNC:
		/* If we were receiving a packet when this tx
		 * started, we drop it now. It will be discarded 
		 * later in endRx.
		 */
		assert (!m_rxing);
		m_rxing = false;
		STATE_FROM (Phy80211::SYNC);
		break;
	case Phy80211::IDLE:
		STATE_FROM (Phy80211::IDLE);
		break;
	default:
		assert (false);
		break;
	}
	m_previousStateChangeTime = now ();
	m_endTx = now () + txDuration;
	STATE_TO (Phy80211::TX);
	STATE_AT (now ());
}
void
Phy80211::switchToSyncFromIdle (double rxDuration)
{
	assert (getState () == Phy80211::IDLE);
	assert (!m_rxing);
	m_previousStateChangeTime = now ();
	m_rxing = true;
	m_endRx = now () + rxDuration;
	assert (getState () == Phy80211::SYNC);
	STATE_FROM (Phy80211::IDLE);
	STATE_TO (Phy80211::SYNC);
	STATE_AT (now ());
}
void
Phy80211::switchToSleep (void)
{
	assert (!m_sleeping);
	switch (getState ()) {
	case Phy80211::SYNC:
		/* If we were receiving a packet when this sleep is
		 * started, we drop it now. It will be discarded 
		 * later in endRx.
		 */
		assert (m_rxing);
		m_rxing = false;
		STATE_FROM (Phy80211::SYNC);
		break;
	case Phy80211::IDLE:
		/* */
		STATE_FROM (Phy80211::IDLE);
		break;
	case Phy80211::TX:
		/* If we were transmitting a packet when this sleep
		 * started, we cannot drop it as we should (obviously,
		 * the transmission will not be able to complete)
		 * because the packet has already been put in the 
		 * reception queue of all the target nodes. To be
		 * able to drop it, we would need to remove it from
		 * each target queue or notify each target to remove 
		 * it.
		 * I know, this sucks and it is a bug but there is no
		 * reasonable fix to it.
		 */
		assert (false);
		break;
	default:
		assert (false);
	}
	m_previousStateChangeTime = now ();
	m_sleeping = true;
	STATE_FROM (Phy80211::SLEEP);
	STATE_AT (now ());
}
void
Phy80211::switchToIdleFromSleep (void)
{
	assert (getState () == Phy80211::SLEEP);
	assert (!m_sleeping);

	m_previousStateChangeTime = now ();
	m_sleeping = false;

	assert (getState () == Phy80211::IDLE);

	STATE_FROM (Phy80211::SLEEP);
	STATE_TO (Phy80211::IDLE);
	STATE_AT (now ());
}
void
Phy80211::switchToIdleFromSync (void)
{
	assert (getState () == Phy80211::SYNC);
	assert (m_rxing);

	m_previousStateChangeTime = now ();
	m_rxing = false;

	assert (getState () == Phy80211::IDLE);

	STATE_FROM (Phy80211::SYNC);
	STATE_TO (Phy80211::IDLE);
	STATE_AT (now ());
}
enum Phy80211::Phy80211State 
Phy80211::getState (void)
{
	if (m_sleeping) {
		assert (m_endTx == 0);
		assert (!m_rxing);
		return Phy80211::SLEEP;
	} else {
		if (m_endTx != 0 && m_endTx > now ()) {
			return Phy80211::TX;
		} else if (m_endTx != 0) {
			/* At one point in the past, we completed
			 * transmission of this packet.
			 */
			STATE_FROM (Phy80211::TX);
			STATE_TO (Phy80211::IDLE);
			STATE_AT (m_endTx);
			
			m_previousStateChangeTime = m_endTx;
			m_endTx = 0;
		}
		if (m_rxing) {
			return Phy80211::SYNC;
		} else {
			return Phy80211::IDLE;
		}
	}
}
double
Phy80211::getStateDuration (void)
{
	return now () - m_previousStateChangeTime;
}
double 
Phy80211::getDelayUntilIdle (void)
{
	double retval;

	switch (getState ()) {
	case SYNC:
		retval = getEndOfRx () - now ();
		break;
	case TX:
		retval = getEndOfTx () - now ();
		break;
	case IDLE:
		retval = 0.0;
		break;
	case SLEEP:
		assert (false);
		// quiet compiler.
		retval = 0.0;
		break;
	}
	retval = max (retval, 0.0);
	return retval;
}
