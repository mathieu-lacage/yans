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

#define nopeMAC_LOW_TRACE 1

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
MacLowReceptionListener::MacLowReceptionListener ()
{}
MacLowReceptionListener::~MacLowReceptionListener ()
{}
MacLowNavListener::MacLowNavListener ()
{}
MacLowNavListener::~MacLowNavListener ()
{}
MacLowBusyMonitoringListener::MacLowBusyMonitoringListener ()
{}
MacLowBusyMonitoringListener::~MacLowBusyMonitoringListener ()
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
	  m_busyMonitorHandler (new DynamicHandler<MacLow> (this, &MacLow::busyMonitor)),
	  m_currentTxPacket (0)
{
	m_lastNavDuration = 0.0;
	m_lastNavStart = 0.0;
	m_overrideDurationId = 0.0;
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
	if (m_monitorBusy) {
		double monitorDelay = txDuration + parameters ()->getPIFS ();
		m_busyMonitorHandler->start (monitorDelay);
	}
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
	if (m_monitorBusy) {
		double monitorDelay = txDuration + parameters ()->getPIFS ();
		m_busyMonitorHandler->start (monitorDelay);
	}
	if (m_waitACK) {
		double timerDelay = txDuration + parameters ()->getACKTimeoutDuration ();
		m_ACKTimeoutHandler->start (new MacCancelableEvent (),
					    timerDelay);
	}
	setTxMode (txPacket, m_dataTxMode);
	double duration;
	if (m_overrideDurationId > 0.0) {
		duration = m_overrideDurationId;
	} else {
		int ackTxMode = getAckTxModeForData (getDestination (txPacket), m_dataTxMode);
		duration = 0.0;
		if (m_waitACK) {
			duration += parameters ()->getSIFS ();
			duration += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
		}
		if (m_nextSize > 0) {
			duration += parameters ()->getSIFS ();
			duration += calculateTxDuration (m_dataTxMode, m_nextSize);
			if (m_waitACK) {
				duration += parameters ()->getSIFS ();
				duration += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
			}
		}
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
MacLow::sendACK_AfterData (class MacCancelableEvent *macEvent)
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
	m_transmissionListener->startNext ();
}

void 
MacLow::busyMonitor (MacCancelableEvent *macEvent)
{
	if (peekPhy ()->getState () == Phy80211::IDLE) {
		m_monitoringListener->gotBusyTimeout ();
	}
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
	/* XXX 
	 * We should be dealing specially with the CF-Poll and CF-End
	 * DurationIDs. Specifically, we need to reset unconditionally
	 * the NAV value in this case. But this should be done
	 * only for QSTAs. Hrm. Damned.
	 * Also, we should consdier the subtile case of RTS/CTS.
	 * See section 9.9.2.2.1, 802.11e/D12.1
	 */

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
		int ackTxMode = getAckTxModeForData (getDestination (m_currentTxPacket), m_dataTxMode);
		txTime += parameters ()->getSIFS ();
		txTime += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
	}
	return txTime;
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
MacLow::enableNextData (int size)
{
	m_nextSize = size;
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
	} else if (isManagement (m_currentTxPacket)) {
		increaseSize (m_currentTxPacket, parameters ()->getMgtHeaderSize ());
	}

	if (m_nextSize > 0 && !m_waitACK) {
		// we need to start the afterSIFS timeout now.
		double delay = calculateOverallTxTime ();
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
double
MacLow::calculateHandshakeDuration (void)
{
	
	return calculateOverallTxTime ();
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

	double monitorDelay = 0.0;

	if (getType (packet) == MAC_80211_CTL_RTS) {
		TRACE ("rx RTS from %d", getSource (packet));
		/* XXX see section 9.9.2.2.1 802.11e/D12.1 */
		if (isPrevNavZero &&
		    getDestination (packet) == getSelf ()) {
			monitorDelay += parameters ()->getSIFS ();
			int ctsTxMode = getCtsTxModeForRts (getSource (packet), getTxMode (packet));
			monitorDelay += calculateTxDuration (ctsTxMode,
							     parameters ()->getCTSSize ());
			monitorDelay += parameters ()->getPIFS ();
			m_sendCTSHandler->start (new SendEvent (packet), parameters ()->getSIFS ());
		} else {
			monitorDelay += parameters ()->getPIFS ();
		}
		dropPacket (packet);
	} else if (getType (packet) == MAC_80211_CTL_CTS &&
		   getDestination (packet) == getSelf () &&
		   m_CTSTimeoutHandler->isRunning () &&
		   m_currentTxPacket) {
		TRACE ("rx CTS from %d", getSource (packet));
		m_CTSTimeoutHandler->cancel ();
		m_transmissionListener->gotCTS (getLastSNR (), getTxMode (packet));
		monitorDelay += parameters ()->getPIFS ();
		m_sendDataHandler->start (new SendEvent (packet), parameters ()->getSIFS ());
		dropPacket (packet);
	} else if (getType (packet) == MAC_80211_CTL_ACK &&
		   getDestination (packet) == getSelf () &&
		   m_ACKTimeoutHandler->isRunning ()) {
		TRACE ("rx ACK from %d", getSource (packet));
		m_ACKTimeoutHandler->cancel ();
		m_transmissionListener->gotACK (getLastSNR (), getTxMode (packet));
		dropPacket (packet);
		if (m_nextSize > 0) {
			m_waitSIFSHandler->start (new MacCancelableEvent (), parameters ()->getSIFS ());
		} else {
			monitorDelay += parameters ()->getPIFS ();
		}
	} else if (isControl (packet)) {
		TRACE ("rx drop %s", getTypeString (packet));
		monitorDelay += parameters ()->getPIFS ();
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
			monitorDelay += parameters ()->getSIFS ();
			int ackTxMode = getAckTxModeForData (getSource (packet), getTxMode (packet));
			monitorDelay += calculateTxDuration (ackTxMode, parameters ()->getACKSize ());
			monitorDelay += parameters ()->getPIFS ();
		} else if (getDestination (packet) == ((int)MAC_BROADCAST)) {
			TRACE ("rx broadcast from %d", getSource (packet));
			m_receptionListener->gotData (packet);
			monitorDelay += parameters ()->getPIFS ();
		} else {
			//TRACE_VERBOSE ("rx not-for-me from %d", getSource (packet));
			dropPacket (packet);
			monitorDelay += parameters ()->getPIFS ();
		}
	}
	if (monitorDelay > 0 && m_monitorBusy) {
		m_busyMonitorHandler->start (monitorDelay);
	}
}



void 
MacLow::enableBusyMonitoring (void)
{
	m_monitorBusy = true;
}
void 
MacLow::disableBusyMonitoring (void)
{
	m_busyMonitorHandler->cancel ();
	m_monitorBusy = false;
}
void 
MacLow::setBusyMonitoringListener (MacLowBusyMonitoringListener *listener)
{
	m_monitoringListener = listener;
}
