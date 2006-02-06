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
#include "rng-uniform.h"
#include "mac-parameters.h"
#include "mac-container.h"

#include <iostream>

#define MAC_TRACE 1

#ifdef MAC_TRACE
# define TRACE(format, ...) \
	printf ("MAC LOW %d %f " format "\n", getSelf (), now (), ## __VA_ARGS__);
#else /* MAC_TRACE */
# define TRACE(format, ...)
#endif /* MAC_TRACE */



MacLowTransmissionListener::MacLowTransmissionListener ()
{}
MacLowTransmissionListener::~MacLowTransmissionListener ()
{}
MacLowReceptionListener::MacLowReceptionListener ()
{}
MacLowReceptionListener::~MacLowReceptionListener ()
{}
MacLowNavListener::MacLowNavListener ()
{}
MacLowNavListener::~MacLowNavListener ()
{}

class SendEvent : public MacCancelableEvent
{
public:
	SendEvent (Packet *packet)
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

MacLow::MacLow (MacContainer *container)
	: m_container (container),
	  m_ACKTimeoutHandler (new DynamicHandler<MacLow> (this, &MacLow::ACKTimeout)),
	  m_CTSTimeoutHandler (new DynamicHandler<MacLow> (this, &MacLow::CTSTimeout)),
	  m_sendCTSHandler (new DynamicHandler<MacLow> (this, &MacLow::sendCTS_AfterRTS)),
	  m_sendACKHandler (new DynamicHandler<MacLow> (this, &MacLow::sendACK_AfterData)),
	  m_sendDataHandler (new DynamicHandler<MacLow> (this, &MacLow::sendDataAfterCTS)),
	  m_waitSIFSHandler (new DynamicHandler<MacLow> (this, &MacLow::waitSIFSAfterEndTx)),
	  m_currentTxPacket (0)
{
	m_additionalDuration = 0.0;
	m_lastNavDuration = 0.0;
	m_lastNavStart = 0.0;
}

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

MacParameters *
MacLow::parameters (void)
{
	return m_container->parameters ();
}

Phy80211 *
MacLow::peekPhy (void)
{
	return m_container->phy ();
}
void
MacLow::forwardDown (Packet *packet)
{
	double txDuration = calculateTxDuration (getTxMode (packet), getSize (packet));
	double durationId = getDuration (packet);
	m_container->forwardToPhy (packet);
	/* Note that it is really important to notify the NAV 
	 * thing after forwarding the packet to the PHY.
	 */
	notifyNav (now ()+txDuration, durationId);
}
int 
MacLow::getSelf (void)
{
	return m_container->selfAddress ();
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
MacLow::isCtl (Packet *packet)
{
	if (getType (packet) == MAC_80211_CTL_RTS     ||
	    getType (packet) == MAC_80211_CTL_CTS     ||
	    getType (packet) == MAC_80211_CTL_ACK     ||
	    getType (packet) == MAC_80211_CTL_BACKREQ ||
	    getType (packet) == MAC_80211_CTL_BACKRESP) {
		return true;
	} else {
		return false;
	}
}

bool
MacLow::isManagement (Packet *packet)
{
	if (isData (packet) || isCtl (packet)) {
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
	setTxMode (packet, m_rtsTxMode);
	double duration = 3 * parameters ()->getSIFS ();
	duration += calculateTxDuration (m_rtsTxMode, parameters ()->getCTSSize ());
	duration += calculateTxDuration (m_dataTxMode, parameters ()->getACKSize ());
	duration += calculateTxDuration (m_dataTxMode, getSize (data));
	duration += m_additionalDuration;
	setDuration (packet, duration);
	return packet;
}



void
MacLow::sendRTSForPacket (Packet *txPacket)
{
	/* send an RTS for this packet. */
	Packet *packet = getRTSforPacket (txPacket);
	TRACE ("tx RTS to %d with mode %d", getDestination (packet), getTxMode (packet));
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
	TRACE ("tx %s to %d with mode %d", getTypeString (txPacket), getDestination (txPacket), m_dataTxMode);
	double txDuration = calculateTxDuration (m_dataTxMode, getSize (txPacket));
	if (m_waitACK) {
		double timerDelay = txDuration + parameters ()->getACKTimeoutDuration ();
		m_ACKTimeoutHandler->start (new MacCancelableEvent (),
					    timerDelay);
	}
	
	setTxMode (txPacket, m_dataTxMode);
	double duration = m_additionalDuration;
	if (m_waitACK) {
		duration += parameters ()->getSIFS ();
		duration += calculateTxDuration (m_dataTxMode, parameters ()->getACKSize ());
	}
	setDuration (txPacket, duration);
	forwardDown (txPacket);
}


void
MacLow::sendCTS_AfterRTS (class MacCancelableEvent *macEvent)
{
	/* send a CTS when you receive a RTS 
	 * right after SIFS.
	 */
	SendEvent *event = static_cast<SendEvent *> (macEvent);
	TRACE ("tx CTS to %d with mode %d", event->getSource (), event->getTxMode ());
	Packet * cts = getCTSPacket ();
	setDestination (cts, event->getSource ());
	double duration = event->getDuration ();
	duration -= calculateTxDuration (event->getTxMode (), getSize (cts));
	duration -= parameters ()->getSIFS ();
	setDuration (cts, duration);
	setTxMode (cts, event->getTxMode ());
	forwardDown (cts);
}

void
MacLow::sendACK_AfterData (class MacCancelableEvent *macEvent)
{
	/* send an ACK when you receive 
	 * a packet after SIFS. 
	 */
	SendEvent *event = static_cast<SendEvent *> (macEvent);
	TRACE ("tx ACK to %d with mode %d", event->getSource (), event->getTxMode ());
	Packet * ack = getACKPacket ();
	setDestination (ack, event->getSource ());
	double duration = event->getDuration ();
	duration -= calculateTxDuration (event->getTxMode (), getSize (ack));
	duration -= parameters ()->getSIFS ();
	setDuration (ack, duration);
	/* use the same tx mode used by the sender. 
	 * XXX: clearly, this is wrong: we should be
	 * using the min of this mode and the BSS mode.
	 */
	setTxMode (ack, event->getTxMode ());
	forwardDown (ack);
}

void
MacLow::sendDataAfterCTS (class MacCancelableEvent *macEvent)
{
	/* send the third step in a 
	 * RTS/CTS/DATA/ACK hanshake 
	 */
	// XXX use sendDataPacket ?
	assert (m_currentTxPacket);
	SendEvent *event = static_cast<SendEvent *> (macEvent);

	TRACE ("tx %s to %d with mode %d", 
	       getTypeString (m_currentTxPacket),
	       getDestination (m_currentTxPacket),
	       m_dataTxMode);
	double txDuration = calculateTxDuration (m_dataTxMode, getSize (m_currentTxPacket));
	if (m_waitACK) {
		double timerDelay = txDuration + parameters ()->getACKTimeoutDuration ();
		m_ACKTimeoutHandler->start (new MacCancelableEvent (),
					    timerDelay);
	}
	
	setTxMode (m_currentTxPacket, m_dataTxMode);
	double duration = event->getDuration ();
	duration -= txDuration;
	duration -= parameters ()->getSIFS ();
	setDuration (m_currentTxPacket, duration);
	forwardDown (m_currentTxPacket);
	m_currentTxPacket = 0;
}


void 
MacLow::waitSIFSAfterEndTx (MacCancelableEvent *macEvent)
{
	m_transmissionListener->txCompletedAndSIFS ();
}



void
MacLow::ACKTimeout (class MacCancelableEvent *event)
{
	m_transmissionListener->missedACK ();
}

void
MacLow::CTSTimeout (class MacCancelableEvent *event)
{
	m_currentTxPacket = 0;
	m_transmissionListener->missedCTS ();
}

bool 
MacLow::isNavZero (double now)
{
	if (m_lastNavStart + m_lastNavDuration > now) {
		return false;
	} else {
		return true;
	}
}

void
MacLow::notifyNav (double now, double duration)
{
	assert (m_lastNavStart < now);
	double oldNavStart = m_lastNavStart;
	double oldNavEnd = oldNavStart + m_lastNavDuration;
	double newNavStart = now;
	double newNavEnd = newNavStart + duration;
	if (oldNavEnd > newNavStart) {
		/* The two NAVs overlap */
		if (newNavEnd > oldNavEnd) {
			double delta = newNavEnd - oldNavEnd;
			m_lastNavDuration += delta;
			vector<MacLowNavListener *>::const_iterator i;
			for (i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
				(*i)->navContinue (delta);
			}
		}
	} else {
		m_lastNavStart = newNavStart;
		m_lastNavDuration = duration;
		vector<MacLowNavListener *>::const_iterator i;
		for (i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
			(*i)->navStart (newNavStart, duration);
		}
	}
}

double
MacLow::calculateOverallTxTime (void)
{
	double txTime = 0.0;
	if (m_sendRTS) {
		txTime += calculateTxDuration (m_rtsTxMode, parameters ()->getRTSSize ());
		txTime += calculateTxDuration (m_rtsTxMode, parameters ()->getCTSSize ());
		txTime += parameters ()->getSIFS () * 2;
	}
	txTime += calculateTxDuration (m_dataTxMode, getSize (m_currentTxPacket));
	if (m_waitACK) {
		txTime += parameters ()->getSIFS ();
		txTime += calculateTxDuration (m_dataTxMode, parameters ()->getACKSize ());
	}
	return txTime;
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
MacLow::setTransmissionListener (MacLowTransmissionListener *listener)
{
	m_transmissionListener = listener;
}
void 
MacLow::setReceptionListener (MacLowReceptionListener *listener)
{
	m_receptionListener = listener;
}
void 
MacLow::setAdditionalDuration (double duration)
{
	m_additionalDuration = duration;
}
double 
MacLow::getDataTransmissionDuration (int size)
{
	return calculateTxDuration (m_dataTxMode, size);
}
void 
MacLow::setDataTransmissionMode (int txMode)
{
	m_dataTxMode = txMode;
}
void 
MacLow::setRtsTransmissionMode (int txMode)
{
	m_rtsTxMode = txMode;
}
void 
MacLow::registerNavListener (MacLowNavListener *listener)
{
	m_navListeners.push_back (listener);
}

void
MacLow::startTransmission (Packet *packet)
{
	assert (peekPhy ()->getState () == Phy80211::IDLE);

	TRACE ("startTx %d to %d", getSize (packet), getDestination (packet));

	if (isData (packet)) {
		increaseSize (packet, parameters ()->getDataHeaderSize ());
	} else if (isManagement (packet)) {
		increaseSize (packet, parameters ()->getMgtHeaderSize ());
	}
	assert (m_currentTxPacket == 0);
	m_currentTxPacket = packet;

	if (m_waitSIFS && !m_waitACK) {
		// we need to start the afterSIFS timeout now.
		double delay = calculateOverallTxTime ();
		delay += parameters ()->getSIFS ();
		m_waitSIFSHandler->start (new MacCancelableEvent (), delay);
	}

	if (m_sendRTS) {
		sendRTSForPacket (packet);
	} else {
		sendDataPacket (packet);
		m_currentTxPacket = 0;
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
		//TRACE_VERBOSE ("rx failed from %d",
		//getSource (packet));
		dropPacket (packet);
		return;
	}
	m_receptionListener->gotPacket (getSource (packet),
					getLastSNR (),
					getTxMode (packet));
	bool isPrevNavZero = isNavZero (now ());
	notifyNav (now (), getDuration (packet));

	if (getType (packet) == MAC_80211_CTL_RTS) {
		TRACE ("rx RTS from %d", getSource (packet));
		if (isPrevNavZero &&
		    getDestination (packet) == getSelf ()) {
			m_sendCTSHandler->start (new SendEvent (packet), parameters ()->getSIFS ());
		}
		dropPacket (packet);
	} else if (getType (packet) == MAC_80211_CTL_CTS &&
		   getDestination (packet) == getSelf () &&
		   m_CTSTimeoutHandler->isRunning () &&
		   m_currentTxPacket) {
		TRACE ("rx CTS from %d", getSource (packet));
		m_CTSTimeoutHandler->cancel ();
		m_transmissionListener->gotCTS (getLastSNR (), getTxMode (packet));
		m_sendDataHandler->start (new SendEvent (packet), parameters ()->getSIFS ());
		dropPacket (packet);
	} else if (getType (packet) == MAC_80211_CTL_ACK &&
		   getDestination (packet) == getSelf () &&
		   m_ACKTimeoutHandler->isRunning ()) {
		TRACE ("rx ACK from %d", getSource (packet));
		m_ACKTimeoutHandler->cancel ();
		m_transmissionListener->gotACK (getLastSNR (), getTxMode (packet));
		dropPacket (packet);
		if (m_waitSIFS) {
			m_waitSIFSHandler->start (new MacCancelableEvent (), parameters ()->getSIFS ());
		}
	} else if (getType (packet) == MAC_80211_CTL_BACKREQ) {
		TRACE ("rx BACK_REQ from %d", getSource (packet));
		Packet *resp = m_receptionListener->gotBlockAckReq (packet);
		// XXX timing is not correct.
		// XXX should use another handler.
		//m_sendDataHandler->start (new SendDataEvent (resp), parameters ()->getSIFS ());
		//dropPacket (packet);
	} else if (isCtl (packet)) {
		TRACE ("rx drop %s", getTypeString (packet));
		dropPacket (packet);
	} else {
		if (isData (packet)) {
			decreaseSize (packet, parameters ()->getDataHeaderSize ());
		} else if (isManagement (packet)) {
			decreaseSize (packet, parameters ()->getMgtHeaderSize ());
		}
		if (getDestination (packet) == getSelf ()) {
			TRACE ("rx unicast from %d", getSource (packet));
			m_sendACKHandler->start (new SendEvent (packet), parameters ()->getSIFS ());
			m_receptionListener->gotData (packet);
		} else if (getDestination (packet) == ((int)MAC_BROADCAST)) {
			TRACE ("rx broadcast from %d", getSource (packet));
			m_receptionListener->gotData (packet);
		} else {
			//TRACE_VERBOSE ("rx not-for-me from %d", getSource (packet));
			dropPacket (packet);
		}
	}
}

