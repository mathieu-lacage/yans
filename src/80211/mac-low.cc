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
#include "backoff.h"
#include "rng-uniform.h"
#include "mac-low-parameters.h"

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


MacLow::MacLow (Mac80211 *mac, MacHigh *high, Phy80211 *phy,
		MacLowParameters *parameters)
	: m_mac (mac),
	  m_high (high),
	  m_phy (phy),
	  m_currentTxPacket (0),
	  m_ACKTimeoutHandler (new DynamicHandler<MacLow> (this, &MacLow::ACKTimeout)),
	  m_CTSTimeoutHandler (new DynamicHandler<MacLow> (this, &MacLow::CTSTimeout)),
	  m_sendCTSHandler (new DynamicHandler<MacLow> (this, &MacLow::sendCTS_AfterRTS)),
	  m_sendACKHandler (new DynamicHandler<MacLow> (this, &MacLow::sendACK_AfterData)),
	  m_sendDataHandler (new DynamicHandler<MacLow> (this, &MacLow::sendDataAfterCTS)),
	  m_parameters (parameters)
{}

MacLow::~MacLow ()
{
	delete m_ACKTimeoutHandler;
	delete m_CTSTimeoutHandler;
	delete m_sendCTSHandler;
	delete m_sendACKHandler;
	delete m_sendDataHandler;
}

/*****************************************************
 *  Accessor methods
 ****************************************************/

MacLowParameters *
MacLow::parameters (void)
{
	return m_parameters;
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

void
MacLow::dropPacket (Packet *packet)
{
	/* XXX: push in drop queue. */
	Packet::free (packet);
}

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








void
MacLow::sendRTSForPacket (Packet *txPacket)
{
	/* send an RTS for this packet. */
	Packet *packet = getRTSforPacket (txPacket);
	TRACE ("tx RTS with mode %d", getTxMode (packet));
	double txDuration = calculateTxDuration (getTxMode (packet), getSize (packet));
	double timerDelay = txDuration + parameters ()->getCTSTimeoutDuration ();
	m_CTSTimeoutHandler->start (new MacCancelableEvent (),
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
	double timerDelay = txDuration + parameters ()->getACKTimeoutDuration ();
	m_ACKTimeoutHandler->start (new MacCancelableEvent (),
				    timerDelay);
	
	setTxMode (txPacket, txMode);
	setDuration (txPacket, parameters ()->getACKTimeoutDuration () + 
		     parameters ()->getSIFS ());
	forwardDown (txPacket);
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
	//dealWithInputQueue ();
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
	//dealWithInputQueue ();
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
	double timerDelay = txDuration + parameters ()->getACKTimeoutDuration ();
	m_ACKTimeoutHandler->start (new MacCancelableEvent (),
				    timerDelay);
	
	setTxMode (m_currentTxPacket, txMode);
	setDuration (m_currentTxPacket, event->getDuration () - txDuration - parameters ()->getSIFS ());
	forwardDown (m_currentTxPacket);
	//dealWithInputQueue ();
}






void
MacLow::ACKTimeout (class MacCancelableEvent *event)
{
	m_listener->missedACK ();
}

void
MacLow::CTSTimeout (class MacCancelableEvent *event)
{
	m_listener->missedCTS ();
}


void
MacLow::notifyNav (double now, double duration)
{
	vector<NavListener *>::const_iterator i;
	for (i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
		(*i)->gotNav (now, duration);
	}
}

/****************************************************************************
 *        API methods below.
 ****************************************************************************/

void 
MacLow::enableWaitForSIFS (void)
{
	m_waitSIFS = true;
}
void 
MacLow::disableWaitForSIFS (void)
{
	m_waitSIFS = false;
}
void 
MacLow::enableACK (void)
{
	m_waitACK = true;
}
void 
MacLow::disableACK (void)
{
	m_waitACK = false;
}
void 
MacLow::enableRTS (void)
{
	m_sendRTS = true;
}
void 
MacLow::disableRTS (void)
{
	m_sendRTS = false;
}
void 
MacLow::setListener (EventListener *listener)
{
	m_listener = listener;
}
void 
MacLow::setTransmissionMode (int txMode)
{
	m_txMode = txMode;
}
void 
MacLow::registerNavListener (NavListener *listener)
{
	m_navListeners.push_back (listener);
}

void
MacLow::startTransmission (Packet *packet)
{
	/* access backoff completed. start a new transmission.
	 */
	assert (peekPhy ()->getState () == Phy80211::IDLE);

	if (isData (packet)) {
		increaseSize (packet, parameters ()->getDataHeaderSize ());
	} else if (isManagement (packet)) {
		increaseSize (packet, parameters ()->getMgtHeaderSize ());
	}
	initialize (packet);
	assert (m_currentTxPacket == 0);
	m_currentTxPacket = packet;

	if (m_sendRTS) {
		sendRTSForPacket (packet);
	} else {
		sendDataPacket (packet);
	}
	/* When this method completes, we have taken ownership of the medium. */
	assert (peekPhy ()->getState () == Phy80211::TX);
}

void 
MacLow::startBlockAckReqTransmission (int to, int tid)
{
	
}


void 
MacLow::receive (Packet *packet)
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
		//dealWithInputQueue ();
		return;
	}
	lookupStation (getSource (packet))->reportRxOk (getLastSNR (),
							 getTxMode (packet));
	bool isPrevNavZero = isNavZero (now ());
	notifyNav (now (), getDuration (packet));

	if (getType (packet) == MAC_80211_CTL_RTS) {
		TRACE ("rx RTS from %d", getSource (packet));
		if (isPrevNavZero &&
		    getDestination (packet) == getSelf ()) {
			m_sendCTSHandler->start (new SendCTSEvent (packet), parameters ()->getSIFS ());
		} else {
			//dealWithInputQueue ();
		}
		dropPacket (packet);
	} else if (getType (packet) == MAC_80211_CTL_CTS &&
		   getDestination (packet) == getSelf () &&
		   m_CTSTimeoutHandler->isRunning () &&
		   m_currentTxPacket) {
		TRACE ("rx CTS from %d", getSource (packet));
		m_CTSTimeoutHandler->cancel ();
		m_listener->gotCTS (getLastSNR ());
		m_sendDataHandler->start (new SendDataEvent (packet), parameters ()->getSIFS ());
		dropPacket (packet);
		//dealWithInputQueue ();
	} else if (getType (packet) == MAC_80211_CTL_ACK &&
		   getDestination (packet) == getSelf () &&
		   m_ACKTimeoutHandler->isRunning () &&
		   m_currentTxPacket) {
		// XXX assert other timers not running.
		TRACE ("rx ACK from %d", getSource (packet));
		m_ACKTimeoutHandler->cancel ();
		m_listener->gotACK (getLastSNR ());
		Packet::free (m_currentTxPacket);
		m_currentTxPacket = 0;
		dropPacket (packet);
		//dealWithInputQueue ();
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
			//dealWithInputQueue ();
		} else {
			TRACE_VERBOSE ("rx not-for-me from %d", getSource (packet));
			dropPacket (packet);
			//dealWithInputQueue ();
		}
	}
}

