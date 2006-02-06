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

#include "mac-low.h"
#include "mac-queue-80211e.h"
#include "hdr-mac-80211.h"
#include "mac-80211.h"
#include "mac-high.h"
#include "phy-80211.h"
#include "mac-station.h"
#include "mac-handler.h"
#include "mac-handler.h"
#include "backoff.h"
#include "rng-uniform.h"
#include "mac-parameters.h"

#include <iostream>

#define nopeMAC_DEBUG 1
#define nopeMAC_TRACE 1
#define nopeMAC_TRACE_VERBOSE 1

#ifdef MAC_DEBUG
# define DEBUG(format, ...) \
	printf ("DEBUG %d " format "\n", getSelf (), ## __VA_ARGS__);
#else /* MAC_DEBUG */
# define DEBUG(format, ...)
#endif /* MAC_DEBUG */

#ifdef MAC_TRACE
# define TRACE(format, ...) \
	printf ("MAC   TERSE %f %d " format "\n", now (), getSelf (), ## __VA_ARGS__);
#else /* MAC_TRACE */
# define TRACE(format, ...)
#endif /* MAC_TRACE */

#ifdef MAC_TRACE_VERBOSE
# define TRACE_VERBOSE(format, ...) \
	printf ("MAC VERBOSE %f %d " format "\n", now (), getSelf (), ## __VA_ARGS__);
#else /* MAC_TRACE_VERBOSE */
# define TRACE_VERBOSE(format, ...)
#endif /* MAC_TRACE_VERBOSE */

class SendCTSEvent : public MacCancelableEvent
{
public:
	SendCTSEvent (Packet *packet)
		: m_source (HDR_MAC_80211 (packet)->getSource ()),
		  m_duration (HDR_MAC_80211 (packet)->getDuration ()),
		  m_txMode (HDR_MAC_80211 (packet)->getTxMode ())
	{}
	int getSource (void)
	{
		return m_source;
	}
	double getDuration (void)
	{
		return m_duration;
	}
	int getTxMode (void)
	{
		return m_txMode;
	}
private:
	int m_source;
	double m_duration;
	int m_txMode;
};

class SendACKEvent : public MacCancelableEvent
{
public:
	SendACKEvent (Packet *packet)
		: m_source (HDR_MAC_80211 (packet)->getSource ()),
		  m_txMode (HDR_MAC_80211 (packet)->getTxMode ())
	{}
	int getSource (void)
	{
		return m_source;
	}
	int getTxMode (void)
	{
		return m_txMode;
	}
private:
	int m_source;
	int m_txMode;
};

class SendDataEvent : public MacCancelableEvent
{
public:
	SendDataEvent (Packet *packet)
		: m_duration (HDR_MAC_80211 (packet)->getDuration ())
	{}
	double getDuration (void)
	{
		return m_duration;
	}
private:
	double m_duration;
};


MacLow::MacLow (Mac80211 *mac, MacHigh *high, Phy80211 *phy)
	: m_currentTxPacket (0),
	  m_mac (mac),
	  m_high (high),
	  m_phy (phy),
	  m_queue (new MacQueue80211e (10.0 /* XXX */)),
	  m_ACKTimeoutBackoffHandler (new DynamicMacHandler (this, &MacLow::ACKTimeout)),
	  m_CTSTimeoutBackoffHandler (new DynamicMacHandler (this, &MacLow::CTSTimeout)),
	  m_accessBackoffHandler (new StaticHandler<MacLow> (this, &MacLow::initialBackoffTimeout)),
	  m_sendCTSHandler (new DynamicMacHandler (this, &MacLow::sendCTS_AfterRTS)),
	  m_sendACKHandler (new DynamicMacHandler (this, &MacLow::sendACK_AfterData)),
	  m_sendDataHandler (new DynamicMacHandler (this, &MacLow::sendDataAfterCTS)),
	  m_backoff (new Backoff (this)),
	  m_random (new RngUniform ())
{
	resetCW ();
	m_SSRC = 0;
	m_SLRC = 0;
	m_sequence = 0;
	phy->registerListener (m_backoff);
}

MacLow::~MacLow ()
{
	delete m_queue;
	delete m_ACKTimeoutBackoffHandler;
	delete m_accessBackoffHandler;
	delete m_sendCTSHandler;
	delete m_sendACKHandler;
	delete m_sendDataHandler;
	delete m_backoff;
	delete m_random;
}

/*****************************************************
 *  Accessor methods
 ****************************************************/

MacParameters *
MacLow::parameters (void)
{
	return m_mac->parameters ();
}

MacStation *
MacLow::lookupStation (int address)
{
	return m_high->lookupStation (address);
}
Phy80211 *
MacLow::peekPhy (void)
{
	return m_phy;
}
void
MacLow::forwardDown (Packet *packet)
{
	HDR_CMN (packet)->direction () = hdr_cmn::DOWN;
	m_mac->downtarget ()->recv (packet, (Handler *)0);
}

int 
MacLow::getSelf (void)
{
	return m_mac->addr ();
}
double 
MacLow::getEIFS (void)
{
	return parameters ()->getEIFS ();
}
double 
MacLow::getDIFS (void)
{
	return parameters ()->getSIFS ();
}
double 
MacLow::getLastSNR (void)
{
	return peekPhy ()->getLastRxSNR ();
}
double 
MacLow::getLastStartRx (void)
{
	return peekPhy ()->getLastRxStartTime ();
}
double
MacLow::calculateTxDuration (int mode, int size)
{
	double duration = peekPhy ()->calculateTxDuration (mode, size);
	return duration;
}
bool
MacLow::isData (Packet *packet)
{
	if (getType (packet) == MAC_80211_DATA) {
		return true;
	} else {
		return false;
	}
}

bool
MacLow::isManagement (Packet *packet)
{
	if (getType (packet) == MAC_80211_DATA ||
	    getType (packet) == MAC_80211_CTL_RTS ||
	    getType (packet) == MAC_80211_CTL_CTS ||
	    getType (packet) == MAC_80211_CTL_ACK) {
		return false;
	} else {
		return true;
	}
}

/****************************************************
 *  Slightly smarter methods below.
 ***************************************************/

double
MacLow::now (void)
{
	double now;
	now = Scheduler::instance ().clock ();
	return now;
}

Packet *
MacLow::getRTSPacket (void)
{
	Packet *packet = Packet::alloc ();
	setSize (packet, parameters ()->getRTSSize ());
	setSource (packet, getSelf ());
	setType (packet, MAC_80211_CTL_RTS);
	return packet;
}
Packet *
MacLow::getCTSPacket (void)
{
	Packet *packet = Packet::alloc ();
	setSize (packet, parameters ()->getCTSSize ());
	setSource (packet, getSelf ());
	setType (packet, MAC_80211_CTL_CTS);
	return packet;
}
Packet *
MacLow::getACKPacket (void)
{
	Packet *packet = Packet::alloc ();
	setSize (packet, parameters ()->getACKSize ());
	setSource (packet, getSelf ());
	setType (packet, MAC_80211_CTL_ACK);
	return packet;
}
Packet *
MacLow::getRTSforPacket (Packet *data)
{
	Packet *packet = getRTSPacket ();
	setSource (packet, getSelf ());
	setDestination (packet, getDestination (data));
	int txMode = lookupStation (getDestination (data))->getRTSMode ();
	setTxMode (packet, txMode);
	double duration = 3 * parameters ()->getSIFS ();
	/* XXX: the duration is calculated as if the RTS 
	 * mode was the same mode used for data payload.
	 * This might not be the case...
	 */
	duration += calculateTxDuration (txMode, parameters ()->getCTSSize ());
	duration += calculateTxDuration (txMode, parameters ()->getACKSize ());
	duration += calculateTxDuration (txMode, getSize (data));
	setDuration (packet, duration);
	return packet;
}

double
MacLow::pickBackoffDelayInCaseOfFailure (void)
{
	int futureCW = calculateNewFailedCW (m_CW);
	double delay = floor (m_random->pick () * futureCW) * parameters ()->getSlotTime ();
	return delay;
}
double
MacLow::pickBackoffDelay (void)
{
	double delay = floor (m_random->pick () * m_CW) * parameters ()->getSlotTime ();
	return delay;
}
void
MacLow::resetCW (void)
{
	m_CW = parameters ()->getCWmin ();
}
int
MacLow::calculateNewFailedCW (int CW)
{
	CW *= 2;
	if (CW > parameters ()->getCWmax ()) {
		CW = parameters ()->getCWmax ();
	}
	return CW;
}
void
MacLow::updateFailedCW (void)
{
	m_CW = calculateNewFailedCW (m_CW);
}

void
MacLow::dropPacket (Packet *packet)
{
	/* XXX: push in drop queue. */
	Packet::free (packet);
}
void
MacLow::dropCurrentTxPacket (void)
{
	/* XXX: push in drop queue. */
	dropPacket (m_currentTxPacket);
	m_currentTxPacket = 0;
}


double 
MacLow::max (double a, double b)
{
	if (a >= b) {
		return a;
	} else {
		return b;
	}
}


double
MacLow::getXIFSLeft (void)
{
	double XIFS;
	if (m_backoff->wasLastRxOk ()) {
		XIFS = parameters ()->getDIFS ();
	}  else {
		XIFS = parameters ()->getEIFS ();
	}
	double lastEvent = max (m_backoff->getLastRxEndTime () + XIFS,
				m_backoff->getLastTxEndTime () + parameters ()->getDIFS ());
	double left = lastEvent - now ();
	left = max (left, 0);
	return left;
}

void
MacLow::increaseSequence (void)
{
	/* see ieee 802.11-1999 7.1.3.4.1 p40 */
	m_sequence++;
	m_sequence %= 4096;
}


/********************************************************
 *  These are helper functions used by the core timers.
 *******************************************************/

void
MacLow::sendRTSForPacket (Packet *txPacket)
{
	/* send an RTS for this packet. */
	Packet *packet = getRTSforPacket (txPacket);
	TRACE ("tx RTS with mode %d", getTxMode (packet));
	double txDuration = calculateTxDuration (getTxMode (packet), getSize (packet));
	double backoffDelay = pickBackoffDelayInCaseOfFailure ();
	double backoffStart = now () + txDuration + parameters ()->getCTSTimeoutDuration ();
	double timerDelay = txDuration + parameters ()->getCTSTimeoutDuration () + backoffDelay;
	m_backoff->start (backoffStart, backoffDelay);
	m_CTSTimeoutBackoffHandler->start (new MacCancelableEvent (),
					   timerDelay);
	forwardDown (packet);
}
void
MacLow::sendDataPacket (Packet *txPacket)
{
	/* send this packet directly. No RTS is needed. */
	setSource (txPacket, getSelf ());
	int txMode = lookupStation (getDestination (txPacket))->getDataMode (getSize (txPacket));
	TRACE ("tx %s to %d with mode %d", getTypeString (txPacket), getDestination (txPacket), txMode);
	double txDuration = calculateTxDuration (txMode, getSize (txPacket));
	double backoffStart = now () + txDuration + parameters ()->getACKTimeoutDuration ();
	double backoffDelay = pickBackoffDelayInCaseOfFailure ();
	double timerDelay = txDuration + parameters ()->getACKTimeoutDuration () + backoffDelay;
	m_backoff->start (backoffStart, backoffDelay);
	m_ACKTimeoutBackoffHandler->start (new MacCancelableEvent (),
					   timerDelay);
	
	setTxMode (txPacket, txMode);
	setDuration (txPacket, parameters ()->getACKTimeoutDuration () + 
		     parameters ()->getSIFS ());
	setSequence (txPacket, m_sequence);
	forwardDown (txPacket);
}
void
MacLow::sendCurrentTxPacket (void)
{
	if (getSize (m_currentTxPacket) > parameters ()->getRTSCTSThreshold ()) {
		sendRTSForPacket (m_currentTxPacket);
	} else {
		sendDataPacket (m_currentTxPacket->copy ());
	}
}
void
MacLow::startTransmission (void)
{
	/* access backoff completed.
	 * start a new transmission.
	 */
	Packet *packet = m_queue->dequeue ();
	increaseSequence ();
	if (isData (packet)) {
		increaseSize (packet, parameters ()->getDataHeaderSize ());
	} else if (isManagement (packet)) {
		increaseSize (packet, parameters ()->getMgtHeaderSize ());
	}
	if (getDestination (packet) == ((int)MAC_BROADCAST)) {
		/* broadcast packets do not require an ACK. */
		int txMode = 0; // XXXX
		TRACE ("tx broadcast (%s) with mode %d", getTypeString (packet), txMode);
		setTxMode (packet, txMode);
		setDuration (packet, 0);
		double backoffStart = now ();
		backoffStart += calculateTxDuration (txMode, getSize (packet));
		backoffStart += parameters ()->getDIFS ();
		m_backoff->start (backoffStart, pickBackoffDelay ());

		forwardDown (packet);
		/* In case there is another packet to send after this 
		 * broadcast packet, we look at the input queue.
		 */
		dealWithInputQueue ();
	} else {
		initialize (packet);
		assert (m_currentTxPacket == 0);
		m_currentTxPacket = packet;
		sendCurrentTxPacket ();
	}
}

void
MacLow::dealWithInputQueue (void)
{
	if (m_queue->isEmpty ()) {
		TRACE_VERBOSE ("deal -- queue empty");
		/* no packet to transmit, nothing to do. */
		return;
	}
	if (m_currentTxPacket) {
		TRACE_VERBOSE ("deal -- txing");
		/* a packet is being txed. we will deal with 
		 * the input queue at the end of the curent 
		 * txtion
		 */
		assert (m_sendDataHandler->isRunning () ||
			m_CTSTimeoutBackoffHandler->isRunning () ||
			m_ACKTimeoutBackoffHandler->isRunning ());
		return;
	}
	if (m_accessBackoffHandler->isRunning ()) {
		TRACE_VERBOSE ("deal -- access timer running");
		/* We are already attempting to send a packet. 
		 * The access timer will deal with the input queue.
		 */
		return;
	}
	if (m_sendCTSHandler->isRunning () ||
	    m_sendACKHandler->isRunning ()) {
		TRACE_VERBOSE ("deal -- cts/ack timer running");
		/* We are answering a handshake either
		 * with a CTS or an ACK.
		 * The sendCTS and sendACK timer handlers
		 * will deal with the input queue.
		 */
		return;
	}
	if (peekPhy ()->getState () == Phy80211::SYNC) {
		TRACE_VERBOSE ("deal -- phy syncing");
		/* The PHY is receiving a packet. Our reception
		 * handler will deal with the input queue.
		 */
		return;
	}
	if (!m_backoff->isCompleted (now ())) {
		/* A previous operation started the backoff counter
		 * but did not start a backoff access timer.
		 * We start the timer now.
		 */
		TRACE_VERBOSE ("deal -- starting backoff timer for %f", 
			       m_backoff->getDelayUntilEnd (now ()));
		m_accessBackoffHandler->start (m_backoff->getDelayUntilEnd (now ()));
		return;
	}
	if (peekPhy ()->getState () == Phy80211::TX) {
		/* The PHY is txing a packet so we have a conflict.
		 * start a backoff timer for our future packet.
		 */
		m_backoff->start (now (), pickBackoffDelay ());
		TRACE_VERBOSE ("deal -- phy txing busy. Starting backoff and timer for %f",
			       m_backoff->getDelayUntilEnd (now ()));
		m_accessBackoffHandler->start (m_backoff->getDelayUntilEnd (now ()));
		return;
	}
	if (!m_backoff->isNavZero (now ())) {
		/* We have a Virtual Carrier Sense Busy status
		 * so we start a timer for end-of-busy + DIFS + backoff
		 */
		m_backoff->start (now (), pickBackoffDelay ());
		TRACE_VERBOSE ("deal -- CS busy. Starting backoff and timer for %f", 
			       m_backoff->getDelayUntilEnd (now ()));
		m_accessBackoffHandler->start (m_backoff->getDelayUntilEnd (now ()));
		return;
	}
	if (getXIFSLeft () > 0.0) {
		/* the DIFS/EIFS period after an end-of-rx 
		 * or end-of-tx has not been completed.
		 */
		TRACE_VERBOSE ("deal -- XIFS period not finished. Starting access timer for %f", 
			       getXIFSLeft ());
		m_accessBackoffHandler->start (getXIFSLeft ());
		return;
	}

	/* We are free to start a transmission now because
	 * we are IDLE and have been IDLE for long enough.
	 */
	startTransmission ();
	return;
}


/********************************************************
 *  The crux of the work is performed in the timers below
 *******************************************************/

void 
MacLow::enqueue (class Packet *packet)
{
	/* A packet is received from the MAC
	 * and the higher-level layers.
	 * We queue it and try to send the
	 * packets already present in the queue.
	 */
	TRACE ("queue");
	m_queue->enqueue (packet);
	dealWithInputQueue ();
}
void
MacLow::flush (void)
{
	Packet *packet;
	packet = m_queue->dequeue ();
	while (packet != NULL) {
		packet = m_queue->dequeue ();
		Packet::free (packet);
	}

}
void 
MacLow::receive (class Packet *packet)
{
	/* A packet is received from the PHY.
	 * When we have handled this packet,
	 * we handle any packet present in the
	 * packet queue.
	 */
	if (HDR_CMN (packet)->error ()) {
		TRACE_VERBOSE ("rx failed from %d",
			       getSource (packet));
		dropPacket (packet);
		dealWithInputQueue ();
		return;
	}
	lookupStation (getSource (packet))->reportRxOk (getLastSNR (),
							 getTxMode (packet));
	bool isNavZero = m_backoff->isNavZero (now ());
	m_backoff->notifyNav (now (), getDuration (packet));

	if (getType (packet) == MAC_80211_CTL_RTS) {
		TRACE ("rx RTS from %d", getSource (packet));
		if (isNavZero &&
		    getDestination (packet) == getSelf ()) {
			m_sendCTSHandler->start (new SendCTSEvent (packet), parameters ()->getSIFS ());
		} else {
			dealWithInputQueue ();
		}
		dropPacket (packet);
	} else if (getType (packet) == MAC_80211_CTL_CTS &&
		   getDestination (packet) == getSelf () &&
		   m_CTSTimeoutBackoffHandler->isRunning () &&
		   m_currentTxPacket &&
		   // XXX getEndTime fucked up.
		   getLastStartRx () <= m_CTSTimeoutBackoffHandler->getEndTime ()) {
		TRACE ("rx CTS from %d", getSource (packet));
		m_CTSTimeoutBackoffHandler->cancel ();
		m_SSRC = 0;
		m_backoff->cancel ();
		m_sendDataHandler->start (new SendDataEvent (packet), parameters ()->getSIFS ());
		lookupStation (getSource (packet))->reportRTSOk (getLastSNR (),
								 getTxMode (packet));
		dropPacket (packet);
		dealWithInputQueue ();
	} else if (getType (packet) == MAC_80211_CTL_ACK &&
		   getDestination (packet) == getSelf () &&
		   m_ACKTimeoutBackoffHandler->isRunning () &&
		   m_currentTxPacket &&
		   // XXX getEndTime fucked up.
		   getLastStartRx () <= m_ACKTimeoutBackoffHandler->getEndTime ()) {
		// XXX assert other timers not running.
		TRACE ("rx ACK from %d", getSource (packet));
		resetCW ();
		m_ACKTimeoutBackoffHandler->cancel ();
		m_SLRC = 0;
		m_backoff->cancel ();
		lookupStation (getSource (packet))->reportDataOk (getLastSNR (),
								 getTxMode (packet));
		m_high->notifyAckReceivedFor (m_currentTxPacket);
		Packet::free (m_currentTxPacket);
		m_currentTxPacket = 0;
		dropPacket (packet);
		// always start a backoff when we correctly receive a packet.
		m_backoff->start (now (), pickBackoffDelay ());
		dealWithInputQueue ();
	} else {
		// data or mgmt packet
		if (isData (packet)) {
			decreaseSize (packet, parameters ()->getDataHeaderSize ());
		} else if (isManagement (packet)) {
			decreaseSize (packet, parameters ()->getMgtHeaderSize ());
		}
		if (getDestination (packet) == getSelf ()) {
			// send ACK.
			m_sendACKHandler->start (new SendACKEvent (packet), parameters ()->getSIFS ());
			
			// filter duplicates
			MacStation *station = lookupStation (getSource (packet));
			if (isRetry (packet) &&
			    getSequence (packet) == station->getLastRxSequence ()) {
				TRACE ("rx DATA duplicate from %d", getSource (packet));
			} else {
				TRACE ("rx DATA from %d", getSource (packet));
				m_high->receiveFromMacLow (packet);
			}
			station->setLastRxSequence (getSequence (packet));
		} else if (getDestination (packet) == ((int)MAC_BROADCAST)) {
			TRACE ("rx broadcast from %d", getSource (packet));
			m_high->receiveFromMacLow (packet);
			dealWithInputQueue ();
		} else {
			TRACE_VERBOSE ("rx not-for-me from %d", getSource (packet));
			dropPacket (packet);
			dealWithInputQueue ();
		}
	}
}

void
MacLow::ACKTimeout (class MacCancelableEvent *event)
{
	if (!m_backoff->isCompleted (now ())) {
		m_ACKTimeoutBackoffHandler->start (new MacCancelableEvent (), 
						   m_backoff->getDelayUntilEnd (now ()));
		TRACE_VERBOSE ("ACK timeout not finished");
		return;
	}
	TRACE_VERBOSE ("ACK timeout SLRC %d", m_SLRC);

	/* access backoff and ACK timeout completed. 
	 * - update retry counters
	 * - update CW
	 * - notify rate control code
	 * - start DATA retry if needed
	 */
	m_SLRC++;
	updateFailedCW ();
	if (m_SLRC > parameters ()->getMaxSLRC ()) {
		/* the transmission of the current packet failed.
		 * the max retry count has been reached.
		 * drop the packet and try to process another one.
		 */
		lookupStation (getDestination (m_currentTxPacket))->reportFinalDataFailed ();
		dropCurrentTxPacket ();
		dealWithInputQueue ();
	} else {
		/* The transmission of the current packet failed.
		 * We re-send it again with a different txMode
		 * and different duration.
		 */
		setRetry (m_currentTxPacket);
		lookupStation (getDestination (m_currentTxPacket))->reportDataFailed ();
		sendCurrentTxPacket ();
	}
}

void
MacLow::CTSTimeout (class MacCancelableEvent *event)
{
	if (!m_backoff->isCompleted (now ())) {
		m_CTSTimeoutBackoffHandler->start (new MacCancelableEvent (),
						   m_backoff->getDelayUntilEnd (now ()));
		TRACE_VERBOSE ("CTS timeout not finished");
		return;
	}
	TRACE_VERBOSE ("CTS timeout");

	/* access backoff and CTS timeout completed. 
	 * - update retry counters
	 * - update CW
	 * - notify rate control code
	 * - start RTS retry if needed
	 */
	m_SSRC++;
	updateFailedCW ();
	if (m_SSRC > parameters ()->getMaxSSRC ()) {
		/* the transmission of the RTS failed.
		 * the max retry count has been reached.
		 * drop the current packet.
		 */
		lookupStation (getDestination (m_currentTxPacket))->reportFinalRTSFailed ();
		dropCurrentTxPacket ();
		dealWithInputQueue ();
	} else {
		/* the transmission of the RTS failed.
		 * re-send the RTS with a new txmode/duration.
		 */
		lookupStation (getDestination (m_currentTxPacket))->reportRTSFailed ();
		sendCurrentTxPacket ();
	}
}
void 
MacLow::initialBackoffTimeout (void)
{
	if (m_queue->isEmpty ()) {
		/* The packet which was queued for transmission
		 * was dropped by the 802.11 queue timeout
		 * before we could try to finish its initial 
		 * backoff.
		 */
		TRACE_VERBOSE ("access timeout with empty queue");
		return;
	}
	if (!m_backoff->isCompleted (now ())) {
		/* start a backoff timer for what is left of 
		 * the backoff.
		 */
		TRACE_VERBOSE ("access timeout not finished");
		m_accessBackoffHandler->start (m_backoff->getDelayUntilEnd (now ()));
		return;
	}
	TRACE_VERBOSE ("access timeout");

	startTransmission ();
}

void
MacLow::sendCTS_AfterRTS (class MacCancelableEvent *macEvent)
{
	/* send a CTS when you receive a RTS 
	 * right after SIFS.
	 */
	SendCTSEvent *event = static_cast<SendCTSEvent *> (macEvent);
	TRACE ("tx CTS with mode %d", event->getTxMode ());
	Packet * cts = getCTSPacket ();
	setDestination (cts, event->getSource ());
	double ctsDuration = calculateTxDuration (event->getTxMode (), getSize (cts));
	setDuration (cts, event->getDuration () - ctsDuration - parameters ()->getSIFS ());
	setTxMode (cts, event->getTxMode ());
	forwardDown (cts);
	dealWithInputQueue ();
}

void
MacLow::sendACK_AfterData (class MacCancelableEvent *macEvent)
{
	/* send an ACK when you receive 
	 * a packet after SIFS. 
	 */
	SendACKEvent *event = static_cast<SendACKEvent *> (macEvent);
	TRACE ("tx ACK to %d with mode %d", event->getSource (), event->getTxMode ());
	Packet * ack = getACKPacket ();
	setDestination (ack, event->getSource ());
	setDuration (ack, 0);
	/* use the same tx mode used by the sender. 
	 * XXX: clearly, this is wrong: we should be
	 * using the min of this mode and the BSS mode.
	 */
	setTxMode (ack, event->getTxMode ());
	forwardDown (ack);
	dealWithInputQueue ();
}

void
MacLow::sendDataAfterCTS (class MacCancelableEvent *macEvent)
{
	/* send the third step in a 
	 * RTS/CTS/DATA/ACK hanshake 
	 */
	// XXX use sendDataPacket ?
	assert (m_currentTxPacket);
	SendDataEvent *event = static_cast<SendDataEvent *> (macEvent);

	int txMode = lookupStation (getDestination (m_currentTxPacket))->getDataMode (getSize (m_currentTxPacket));
	TRACE ("tx %s to %d with mode %d", 
	       getTypeString (m_currentTxPacket),
	       getDestination (m_currentTxPacket),
	       txMode);
	double txDuration = calculateTxDuration (txMode, getSize (m_currentTxPacket));
	double backoffStart = now () + txDuration + parameters ()->getACKTimeoutDuration ();
	double backoffDelay = pickBackoffDelayInCaseOfFailure ();
	double timerDelay = txDuration + parameters ()->getACKTimeoutDuration () + backoffDelay;
	m_backoff->start (backoffStart, backoffDelay);
	m_ACKTimeoutBackoffHandler->start (new MacCancelableEvent (),
					   timerDelay);
	
	setTxMode (m_currentTxPacket, txMode);
	setDuration (m_currentTxPacket, event->getDuration () - txDuration - parameters ()->getSIFS ());
	setSequence (m_currentTxPacket, m_sequence);
	forwardDown (m_currentTxPacket->copy ());
	dealWithInputQueue ();
}
