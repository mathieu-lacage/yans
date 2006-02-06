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
#include "mac-traces.h"

#include <iostream>

#ifndef MAC_LOW_TRACE
#define MAC_LOW_TRACE 1
#endif

#ifdef MAC_LOW_TRACE
# define TRACE(format, ...) \
	printf ("MAC LOW %d %f " format "\n", getSelf (), now (), ## __VA_ARGS__);
#else /* MAC_LOW_TRACE */
# define TRACE(format, ...)
#endif /* MAC_LOW_TRACE */



MacLowTransmissionListener::MacLowTransmissionListener ()
{}
MacLowTransmissionListener::~MacLowTransmissionListener ()
{}
NullMacLowTransmissionListener::NullMacLowTransmissionListener ()
{}
NullMacLowTransmissionListener::~NullMacLowTransmissionListener ()
{}
void NullMacLowTransmissionListener::gotCTS (double snr, int txMode)
{}
void NullMacLowTransmissionListener::missedCTS (void)
{}
void NullMacLowTransmissionListener::gotACK (double snr, int txMode)
{}
void NullMacLowTransmissionListener::missedACK (void)
{}
void NullMacLowTransmissionListener::startNext (void)
{}
MacLowTransmissionListener *NullMacLowTransmissionListener::m_instance = 0;
MacLowTransmissionListener *
NullMacLowTransmissionListener::instance (void)
{
	if (m_instance == 0) {
		m_instance = new NullMacLowTransmissionListener ();
	}
	return m_instance;
}
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
	  m_normalAckTimeoutHandler (new DynamicHandler<MacLow> (this, &MacLow::normalAckTimeout)),
	  m_fastAckTimeoutHandler (new StaticHandler<MacLow> (this, &MacLow::fastAckTimeout)),
	  m_superFastAckTimeoutHandler (new StaticHandler<MacLow> (this, &MacLow::superFastAckTimeout)),
	  m_fastAckFailedTimeoutHandler (new StaticHandler<MacLow> (this, &MacLow::fastAckFailedTimeout)),
	  m_CTSTimeoutHandler (new DynamicHandler<MacLow> (this, &MacLow::CTSTimeout)),
	  m_sendCTSHandler (new DynamicHandler<MacLow> (this, &MacLow::sendCTS_AfterRTS)),
	  m_sendACKHandler (new DynamicHandler<MacLow> (this, &MacLow::sendACK_AfterData)),
	  m_sendDataHandler (new DynamicHandler<MacLow> (this, &MacLow::sendDataAfterCTS)),
	  m_waitSIFSHandler (new DynamicHandler<MacLow> (this, &MacLow::waitSIFSAfterEndTx)),
	  m_currentTxPacket (0)
{
	m_lastNavDuration = 0.0;
	m_lastNavStart = 0.0;
	m_overrideDurationId = 0.0;
}

MacLow::~MacLow ()
{
	delete m_normalAckTimeoutHandler;
	delete m_fastAckTimeoutHandler;
	delete m_fastAckFailedTimeoutHandler;
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
	enum mac_80211_packet_type type = getType (packet);
	m_container->forwardToPhy (packet);
	/* Note that it is really important to notify the NAV 
	 * thing _after_ forwarding the packet to the PHY.
	 * but we must perform the calculations _before_ passing
	 * the thing to the PHY (I think I remember the PHY does
	 * some nasty things to the underlying memory of the 
	 * packet)
	 */
	notifyNav (now ()+txDuration, durationId, type, getSelf ());
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
MacLow::calculateTxDuration (int mode, uint32_t size)
{
	double duration = peekPhy ()->calculateTxDuration (mode, size);
	return duration;
}

void
MacLow::dropPacket (Packet *packet)
{
	/* XXX: push in drop queue. */
	Packet::free (packet);
}

int
MacLow::getCtsTxModeForRts (int to, int rtsTxMode)
{
	return rtsTxMode;
}
int 
MacLow::getAckTxModeForData (int to, int dataTxMode)
{
	/* XXX: clearly, this is not very efficient: 
	 * we should be using the min of this mode 
	 * and the BSS mode.
	 */
	return 0;
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
	Packet *packet = hdr_mac_80211::create (getSelf ());
	setSize (packet, parameters ()->getRTSSize ());
	setType (packet, MAC_80211_CTL_RTS);
	return packet;
}
Packet *
MacLow::getCTSPacket (void)
{
	Packet *packet = hdr_mac_80211::create (getSelf ());
	setSize (packet, parameters ()->getCTSSize ());
	setType (packet, MAC_80211_CTL_CTS);
	return packet;
}
Packet *
MacLow::getACKPacket (void)
{
	Packet *packet = hdr_mac_80211::create (getSelf ());
	setSize (packet, parameters ()->getACKSize ());
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
	double duration;
	if (m_overrideDurationId > 0.0) {
		duration = m_overrideDurationId;
	} else {
		int ackTxMode = getAckTxModeForData (getDestination (data), m_dataTxMode);
		duration = 0.0;
		duration += parameters ()->getSIFS ();
		duration += calculateTxDuration (m_rtsTxMode, parameters ()->getCTSSize ());
		duration += parameters ()->getSIFS ();
		duration += calculateTxDuration (m_dataTxMode, getSize (data));
		duration += parameters ()->getSIFS ();
		duration += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
	}
	setDuration (packet, duration);
	return packet;
}



void
MacLow::sendRTSForPacket (void)
{
	/* send an RTS for this packet. */
	Packet *packet = getRTSforPacket (m_currentTxPacket);
	TRACE ("tx RTS to %d with mode %d", getDestination (packet), getTxMode (packet));
	double txDuration = calculateTxDuration (getTxMode (packet), getSize (packet));
	double timerDelay = txDuration + parameters ()->getCTSTimeoutDuration ();
	m_CTSTimeoutHandler->start (new MacCancelableEvent (),
				    timerDelay);
	forwardDown (packet);
}
void
MacLow::sendDataPacket (void)
{
	/* send this packet directly. No RTS is needed. */
	Packet *txPacket = m_currentTxPacket;
	setSource (txPacket, getSelf ());
	TRACE ("tx %s to %d with mode %d", getTypeString (txPacket), getDestination (txPacket), m_dataTxMode);
	double txDuration = calculateTxDuration (m_dataTxMode, getSize (txPacket));
	if (waitNormalAck ()) {
		double timerDelay = txDuration + parameters ()->getACKTimeoutDuration ();
		m_normalAckTimeoutHandler->start (timerDelay);
	} else if (waitFastAck ()) {
		double timerDelay = txDuration + parameters ()->getPIFS ();
		m_fastAckTimeoutHandler->start (timerDelay);
	} else if (waitSuperFastAck ()) {
		double timerDelay = txDuration + parameters ()->getPIFS ();
		m_superFastAckTimeoutHandler->start (timerDelay);
	}
	setTxMode (txPacket, m_dataTxMode);
	double duration;
	if (m_overrideDurationId > 0.0) {
		duration = m_overrideDurationId;
	} else {
		int ackTxMode = getAckTxModeForData (getDestination (txPacket), m_dataTxMode);
		duration = 0.0;
		if (waitAck ()) {
			duration += parameters ()->getSIFS ();
			duration += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
		}
		if (m_nextSize > 0) {
			duration += parameters ()->getSIFS ();
			duration += calculateTxDuration (m_nextTxMode, m_nextSize);
			if (waitAck ()) {
				duration += parameters ()->getSIFS ();
				duration += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
			}
		}
	}
	setDuration (txPacket, duration);
	forwardDown (txPacket);
}


void
MacLow::sendCTS_AfterRTS (MacCancelableEvent *macEvent)
{
	/* send a CTS when you receive a RTS 
	 * right after SIFS.
	 */
	SendEvent *event = static_cast<SendEvent *> (macEvent);
	int ctsTxMode = getCtsTxModeForRts (event->getSource (), event->getTxMode ());
	TRACE ("tx CTS to %d with mode %d", event->getSource (), ctsTxMode);
	Packet * cts = getCTSPacket ();
	setDestination (cts, event->getSource ());
	double duration = event->getDuration ();
	duration -= calculateTxDuration (ctsTxMode, getSize (cts));
	duration -= parameters ()->getSIFS ();
	setDuration (cts, duration);
	setTxMode (cts, ctsTxMode);
	forwardDown (cts);
}

void
MacLow::sendACK_AfterData (MacCancelableEvent *macEvent)
{
	/* send an ACK when you receive 
	 * a packet after SIFS. 
	 */
	SendEvent *event = static_cast<SendEvent *> (macEvent);
	int ackTxMode = getAckTxModeForData (event->getSource (), event->getTxMode ());
	TRACE ("tx ACK to %d with mode %d", event->getSource (), ackTxMode);
	Packet * ack = getACKPacket ();
	setDestination (ack, event->getSource ());
	double duration = event->getDuration ();
	duration -= calculateTxDuration (ackTxMode, getSize (ack));
	duration -= parameters ()->getSIFS ();
	setDuration (ack, duration);
	setTxMode (ack, ackTxMode);
	forwardDown (ack);
}

void
MacLow::sendDataAfterCTS (MacCancelableEvent *macEvent)
{
	/* send the third step in a 
	 * RTS/CTS/DATA/ACK hanshake 
	 */
	// XXX use sendDataPacket ?
	assert (m_currentTxPacket);
	SendEvent *event = static_cast<SendEvent *> (macEvent);

	TRACE ("tx %s to %d with mode %d seq=0x%x tid=%u", 
	       getTypeString (m_currentTxPacket),
	       getDestination (m_currentTxPacket),
	       m_dataTxMode, 
	       getSequenceControl (m_currentTxPacket),
	       getTID (m_currentTxPacket));
	double txDuration = calculateTxDuration (m_dataTxMode, getSize (m_currentTxPacket));
	if (waitNormalAck ()) {
		double timerDelay = txDuration + parameters ()->getACKTimeoutDuration ();
		m_normalAckTimeoutHandler->start (timerDelay);
	} else if (waitFastAck ()) {
		double timerDelay = txDuration + parameters ()->getPIFS ();
		m_fastAckTimeoutHandler->start (timerDelay);
	} else if (waitSuperFastAck ()) {
		double timerDelay = txDuration + parameters ()->getPIFS ();
		m_superFastAckTimeoutHandler->start (timerDelay);
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
	m_transmissionListener->startNext ();
}

void
MacLow::normalAckTimeout (MacCancelableEvent *event)
{
	m_transmissionListener->missedACK ();
}
void
MacLow::fastAckFailedTimeout (void)
{
	m_transmissionListener->missedACK ();
	TRACE ("fast Ack busy but missed");
}
void
MacLow::fastAckTimeout ()
{
	if (peekPhy ()->getState () == Phy80211::IDLE) {
		TRACE ("fast Ack idle missed");
		m_transmissionListener->missedACK ();
	}
}
void
MacLow::superFastAckTimeout ()
{
	if (peekPhy ()->getState () == Phy80211::IDLE) {
		TRACE ("super fast Ack failed");
		m_transmissionListener->missedACK ();
	} else {
		TRACE ("super fast Ack ok");
		m_transmissionListener->gotACK (0.0, 0);
	}
}

void
MacLow::CTSTimeout (MacCancelableEvent *event)
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
MacLow::notifyNav (double nowTime, double duration, 
		   enum mac_80211_packet_type type,
		   int source)
{
	/* XXX
	 * We might need to do something special for the
	 * subtle case of RTS/CTS. I don't know what.
	 *
	 * See section 9.9.2.2.1, 802.11e/D12.1
	 */
	assert (m_lastNavStart < nowTime);
	double oldNavStart = m_lastNavStart;
	double oldNavEnd = oldNavStart + m_lastNavDuration;
	double newNavStart = nowTime;
	double newNavEnd = newNavStart + duration;

	if (type == MAC_80211_MGT_CFPOLL &&
	    source == m_container->getBSSID ()) {
		m_lastNavStart = newNavStart;
		m_lastNavDuration = duration;
		for (NavListenerCI i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
			(*i)->navReset (newNavStart);
		}
		return;
	}

	if (oldNavEnd > newNavStart) {
		/* The two NAVs overlap */
		if (newNavEnd > oldNavEnd) {
			double delta = newNavEnd - oldNavEnd;
			m_lastNavDuration += delta;
			for (NavListenerCI i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
				(*i)->navContinue (delta);
			}
		}
	} else {
		m_lastNavStart = newNavStart;
		m_lastNavDuration = duration;
		for (NavListenerCI i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
			(*i)->navStart (newNavStart, duration);
		}
	}
}

double
MacLow::calculateOverallCurrentTxTime (void)
{
	double txTime = 0.0;
	if (m_sendRTS) {
		txTime += calculateTxDuration (m_rtsTxMode, parameters ()->getRTSSize ());
		txTime += calculateTxDuration (m_rtsTxMode, parameters ()->getCTSSize ());
		txTime += parameters ()->getSIFS () * 2;
	}
	txTime += calculateTxDuration (m_dataTxMode, getSize (m_currentTxPacket));
	if (waitAck ()) {
		int ackTxMode = getAckTxModeForData (getDestination (m_currentTxPacket), m_dataTxMode);
		txTime += parameters ()->getSIFS ();
		txTime += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
	}
	return txTime;
}

bool
MacLow::waitAck (void)
{
	if (m_waitAck == ACK_NONE) {
		return false;
	} else {
		return true;
	}
}
bool
MacLow::waitNormalAck (void)
{
	if (m_waitAck == ACK_NORMAL) {
		return true;
	} else {
		return false;
	}
}
bool
MacLow::waitFastAck (void)
{
	if (m_waitAck == ACK_FAST) {
		return true;
	} else {
		return false;
	}
}
bool
MacLow::waitSuperFastAck (void)
{
	if (m_waitAck == ACK_SUPER_FAST) {
		return true;
	} else {
		return false;
	}
}

/****************************************************************************
 *        API methods below.
 ****************************************************************************/

void 
MacLow::setData (Packet *packet)
{
	assert (m_currentTxPacket == 0);
	m_currentTxPacket = packet;
}
void
MacLow::clearData (void)
{
	m_currentTxPacket = 0;
}

void 
MacLow::enableNextData (uint32_t size, int txMode)
{
	m_nextSize = size;
	m_nextTxMode = txMode;
}
void 
MacLow::disableNextData (void)
{
	m_nextSize = 0;
}
void 
MacLow::enableOverrideDurationId (double durationId)
{
	m_overrideDurationId = durationId;
}
void 
MacLow::disableOverrideDurationId (void)
{
	m_overrideDurationId = 0.0;
}

void 
MacLow::startTransmission (void)
{
	assert (peekPhy ()->getState () == Phy80211::IDLE);

	TRACE ("startTx %d to %d", getSize (m_currentTxPacket), getDestination (m_currentTxPacket));

	if (isData (m_currentTxPacket)) {
		increaseSize (m_currentTxPacket, parameters ()->getDataHeaderSize ());
	}

	if (m_nextSize > 0 && !waitAck ()) {
		// we need to start the afterSIFS timeout now.
		double delay = calculateOverallCurrentTxTime ();
		delay += parameters ()->getSIFS ();
		m_waitSIFSHandler->start (new MacCancelableEvent (), delay);
	}

	if (m_sendRTS) {
		sendRTSForPacket ();
	} else {
		sendDataPacket ();
		m_currentTxPacket = 0;
	}
	/* When this method completes, we have taken ownership of the medium. */
	assert (peekPhy ()->getState () == Phy80211::TX);	
}

void 
MacLow::enableSuperFastAck (void)
{
	m_waitAck = ACK_SUPER_FAST;
}
void 
MacLow::enableFastAck (void)
{
	m_waitAck = ACK_FAST;
}
void 
MacLow::enableACK (void)
{
	m_waitAck = ACK_NORMAL;
}
void 
MacLow::disableACK (void)
{
	m_waitAck = ACK_NONE;
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
MacLow::receive (Packet *packet)
{
	/* A packet is received from the PHY.
	 * When we have handled this packet,
	 * we handle any packet present in the
	 * packet queue.
	 */
	if (HDR_CMN (packet)->error ()) {
		TRACE ("rx failed from %d",
		       getSource (packet));
		dropPacket (packet);
		if (waitFastAck ()) {
			m_fastAckFailedTimeoutHandler->start (parameters ()->getSIFS ());
		}
		goto out;
	}
	m_receptionListener->gotPacket (getSource (packet),
					getLastSNR (),
					getTxMode (packet));
	bool isPrevNavZero = isNavZero (now ());
	TRACE ("duration/id: %f", getDuration (packet));
	notifyNav (now (), getDuration (packet), getType (packet), getSource (packet));


	if (getType (packet) == MAC_80211_CTL_RTS) {
		/* XXX see section 9.9.2.2.1 802.11e/D12.1 */
		if (isPrevNavZero &&
		    getDestination (packet) == getSelf ()) {
			TRACE ("rx RTS from %d -- schedule CTS", getSource (packet));
			m_sendCTSHandler->start (new SendEvent (packet), parameters ()->getSIFS ());
		} else {
			TRACE ("rx RTS from %d -- cannot schedule CTS", getSource (packet));
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
		   waitAck ()) {
		TRACE ("rx ACK from %d", getSource (packet));
		if (waitNormalAck ()) {
			m_normalAckTimeoutHandler->cancel ();
		}
		if (waitNormalAck () || waitFastAck ()) {
			m_transmissionListener->gotACK (getLastSNR (), getTxMode (packet));
		}
		dropPacket (packet);
		if (m_nextSize > 0) {
			m_waitSIFSHandler->start (new MacCancelableEvent (), parameters ()->getSIFS ());
		}
	} else if (isControl (packet)) {
		TRACE ("rx drop %s", getTypeString (packet));
		dropPacket (packet);
	} else {
		if (isData (packet)) {
			decreaseSize (packet, parameters ()->getDataHeaderSize ());
		}
		if (getDestination (packet) == getSelf ()) {
			if (!isNoAck (packet)) {
				TRACE ("rx unicast/send_ack from %d", getSource (packet));
				m_sendACKHandler->start (new SendEvent (packet), parameters ()->getSIFS ());
			} else {
				TRACE ("rx unicast/no_ack from %d", getSource (packet));
			}
			m_receptionListener->gotData (packet);
		} else if (getDestination (packet) == ((int)MAC_BROADCAST)) {
			TRACE ("rx broadcast from %d", getSource (packet));
			m_receptionListener->gotData (packet);
		} else {
			//TRACE_VERBOSE ("rx not-for-me from %d", getSource (packet));
			dropPacket (packet);
		}
	}
 out:
	return;
}



