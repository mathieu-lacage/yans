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

// ns header.
#include "packet.h"

// 802.11 headers.
#include "edca-txop.h"
#include "dcf.h"
#include "mac-parameters.h"
#include "mac-dcf-parameters.h"
#include "mac-low.h"
#include "mac-queue-80211e.h"
#include "mac-station.h"
#include "mac-stations.h"
#include "mac-high.h"
#include "mac-container.h"
#include "mac-traces.h"
#include "mac-tx-middle.h"


#ifndef EDCA_TXOP_TRACE
#define EDCA_TXOP_TRACE 1
#endif /* EDCA_TXOP_TRACE */

#ifdef EDCA_TXOP_TRACE
# define TRACE(format, ...) \
  printf ("EDCA TXOP %d %f " format "\n", m_container->selfAddress (), Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* EDCA_TXOP_TRACE */
# define TRACE(format, ...)
#endif /* EDCA_TXOP_TRACE */


class MyEdcaAccessListener : public DcfAccessListener {
public:
	MyEdcaAccessListener (EdcaTxop *txop)
		: DcfAccessListener (),
		  m_txop (txop) {}

	virtual ~MyEdcaAccessListener () {}

	virtual void accessGrantedNow (void)
	{
		m_txop->accessGrantedNow ();
	}
	virtual bool accessNeeded (void)
	{
		return m_txop->accessNeeded ();
	}
	virtual bool accessingAndWillNotify (void)
	{
		return m_txop->accessingAndWillNotify ();
	}

private:
	EdcaTxop *m_txop;
};

class MyEdcaTransmissionListener : public MacLowTransmissionListener {
public:
	MyEdcaTransmissionListener (EdcaTxop *txop)
		: MacLowTransmissionListener (),
		  m_txop (txop) {}
		  
	virtual ~MyEdcaTransmissionListener () {}

	virtual void gotCTS (double snr, int txMode) {
		m_txop->gotCTS (snr, txMode);
	}
	virtual void missedCTS (void) {
		m_txop->missedCTS ();
	}
	virtual void gotACK (double snr, int txMode) {
		m_txop->gotACK (snr, txMode);
	}
	virtual void missedACK (void) {
		m_txop->missedACK ();
	}
	virtual void startNext (void) {
		m_txop->startNext ();
	}
	virtual void cancel (void) {
		m_txop->cancel ();
	}

private:
	EdcaTxop *m_txop;
};


EdcaTxop::EdcaTxop (Dcf *dcf, MacQueue80211e *queue, MacContainer *container)
	: m_dcf (dcf),
	  m_queue (queue),
	  m_container (container),
	  m_currentTxPacket (0),
	  m_SSRC (0),
	  m_SLRC (0)
{
	m_dcf->registerAccessListener (new MyEdcaAccessListener (this));
	m_transmissionListener = new MyEdcaTransmissionListener (this);
}

double
EdcaTxop::now (void)
{
	return Scheduler::instance ().clock ();
}

MacParameters *
EdcaTxop::parameters (void)
{
	return m_container->parameters ();
}

MacLow *
EdcaTxop::low (void)
{
	return m_container->macLow ();
}

bool
EdcaTxop::needRTS (void)
{
	if (getSize (m_currentTxPacket) > parameters ()->getRTSCTSThreshold ()) {
		return true;
	} else {
		return false;
	}
}


bool 
EdcaTxop::accessingAndWillNotify (void)
{
	if (m_currentTxPacket != 0) {
		return true;
	} else {
		return false;
	}
}

bool 
EdcaTxop::accessNeeded (void)
{
	if (!m_queue->isEmpty () ||
	    m_currentTxPacket != 0) {
		TRACE ("access needed here");
		return true;
	} else {
		TRACE ("no access needed here -- %d/%p", m_queue->isEmpty (), m_currentTxPacket);
		return false;
	}
}

double
EdcaTxop::calculateTxDuration (uint32_t size, int txMode)
{
	return m_container->phy ()->calculateTxDuration (txMode, size);
}

double
EdcaTxop::calculateCurrentTxDuration (void)
{
	double txDuration = 0.0;
	if (getDestination (m_currentTxPacket) == (int)MAC_BROADCAST) {
		txDuration += calculateTxDuration (getSize (m_currentTxPacket), 0);
	} else {
		int dataTxMode = lookupDestStation (m_currentTxPacket)->getDataMode (getSize (m_currentTxPacket));
		if (m_firstPacketInBurst && needRTS ()) {
			int rtsTxMode = lookupDestStation (m_currentTxPacket)->getRTSMode ();
			txDuration += parameters ()->getSIFS () * 2;
			txDuration += calculateTxDuration (parameters ()->getPacketSize (MAC_80211_CTL_RTS), rtsTxMode);
			txDuration += calculateTxDuration (parameters ()->getPacketSize (MAC_80211_CTL_CTS), 0);
		}
		txDuration += calculateTxDuration (getSize (m_currentTxPacket), dataTxMode);
		txDuration += parameters ()->getSIFS ();
		txDuration += calculateTxDuration (parameters ()->getPacketSize (MAC_80211_CTL_ACK), 0);
	}
	return txDuration;
}
double
EdcaTxop::calculateNextTxDuration (Packet *packet)
{
	double txDuration = 0.0;
	if (getDestination (packet) == (int)MAC_BROADCAST) {
		txDuration += calculateTxDuration (getSize (packet), 0);
	} else {
		int dataTxMode = lookupDestStation (packet)->getDataMode (getSize (packet));
		txDuration += calculateTxDuration (getSize (packet), dataTxMode);
		txDuration += parameters ()->getSIFS ();
		txDuration += calculateTxDuration (parameters ()->getPacketSize (MAC_80211_CTL_ACK), 0);
	}
	return txDuration;
}

double
EdcaTxop::enoughCurrentTime (void)
{
	if (enoughTime (calculateCurrentTxDuration ()) ||
	    m_dcf->parameters ()->getTxopLimit () == 0.0) {
		return true;
	} else {
		return false;
	}
}
double
EdcaTxop::enoughTime (double txDuration)
{
	double endTxTime = now () + txDuration;
	double endTxopTime = m_startTxopTime + m_dcf->parameters ()->getTxopLimit ();
	if (endTxTime > endTxopTime) {
		return false;
	} else {
		return true;
	}
}

void
EdcaTxop::accessGrantedNow (void)
{
	m_firstPacketInBurst = true;
	m_startTxopTime = now ();
	tryToSendOnePacket ();
}

void
EdcaTxop::tryToSendOnePacket (void)
{
	if (!m_currentTxPacket) {
		if (m_queue->isEmpty ()) {
			TRACE ("queue empty");
			return;
		}
		m_currentTxPacket = m_queue->dequeue ();
		initialize (m_currentTxPacket);
		assert (m_currentTxPacket != 0);
		uint16_t sequence = m_container->macTxMiddle ()->getNextSequenceNumberFor (m_currentTxPacket);
		setSequenceNumber (m_currentTxPacket, sequence);
		m_SSRC = 0;
		m_SLRC = 0;
		TRACE ("dequeued %d to %d seq: 0x%x", 
		       getSize (m_currentTxPacket), 
		       getDestination (m_currentTxPacket),
		       getSequenceControl (m_currentTxPacket));
	}
	if (!enoughCurrentTime ()) {
		TRACE ("no time to start transmission.");
		return;
	}

	if (!m_queue->isEmpty () &&
	    m_dcf->parameters ()->getTxopLimit () > 0.0) {
		Packet *packet = m_queue->peekNextPacket ();
		int nextTxMode = lookupDestStation (packet)->getDataMode (getSize (packet));
		double txDuration = calculateNextTxDuration (packet);
		txDuration += calculateCurrentTxDuration ();
		if (enoughTime (txDuration)) {
			m_lastPacketInBurst = false;
			low ()->enableNextData (getSize (packet), nextTxMode);
		} else {
			m_lastPacketInBurst = true;
			low ()->disableNextData ();
		}
	} else {
		m_lastPacketInBurst = true;
		low ()->disableNextData ();
	}

	low ()->disableOverrideDurationId ();
	if (getDestination (m_currentTxPacket) == (int)MAC_BROADCAST) {
		low ()->disableRTS ();
		low ()->disableACK ();
		low ()->setDataTransmissionMode (0);
		low ()->setTransmissionListener (m_transmissionListener);
		low ()->setData (m_currentTxPacket);
		m_currentTxPacket = 0;
		m_dcf->notifyAccessOngoingOk ();
		if (m_lastPacketInBurst) {
			m_dcf->notifyAccessFinished ();
		}
		TRACE ("tx broadcast");
	} else {
		int dataTxMode = lookupDestStation (m_currentTxPacket)->getDataMode (getSize (m_currentTxPacket));
		low ()->setDataTransmissionMode (dataTxMode);
		low ()->enableACK ();
		low ()->setTransmissionListener (m_transmissionListener);
		if (m_firstPacketInBurst && needRTS ()) {
			low ()->enableRTS ();
			int txMode = lookupDestStation (m_currentTxPacket)->getRTSMode ();
			low ()->setRtsTransmissionMode (txMode);
			TRACE ("tx unicast rts mode %d/%d", txMode, dataTxMode);
		} else {
			low ()->disableRTS ();
			TRACE ("tx unicast mode %d", dataTxMode);
		}
		low ()->setData (m_currentTxPacket->copy ());
	}
	low ()->startTransmission ();

}


void 
EdcaTxop::gotCTS (double snr, int txMode)
{
	TRACE ("got cts");
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportRTSOk (snr, txMode);
	m_SSRC = 0;
}
void 
EdcaTxop::missedCTS (void)
{
	TRACE ("missed cts at %f", now ());
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportRTSFailed ();
	m_SSRC++;
	if (m_SSRC > parameters ()->getMaxSSRC ()) {
		station->reportFinalRTSFailed ();
		// to reset the dcf.
		m_dcf->notifyAccessOngoingErrorButOk ();
		dropCurrentPacket ();
	} else {
		m_dcf->notifyAccessOngoingError ();
	}
	m_dcf->notifyAccessFinished ();
}
void 
EdcaTxop::gotACK (double snr, int txMode)
{
	TRACE ("got ack");
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataOk (snr, txMode);
	m_SLRC = 0;

	m_container->macHigh ()->notifyAckReceivedFor (m_currentTxPacket);
	Packet::free (m_currentTxPacket);
	m_currentTxPacket = 0;
	m_dcf->notifyAccessOngoingOk ();

	if (m_lastPacketInBurst) {
		m_dcf->notifyAccessFinished ();
	}
}
void 
EdcaTxop::missedACK (void)
{
	TRACE ("missed ack");
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataFailed ();
	m_SLRC++;
	if (m_SLRC > parameters ()->getMaxSLRC ()) {
		station->reportFinalDataFailed ();
		// to reset the dcf.
		m_dcf->notifyAccessOngoingErrorButOk ();
		dropCurrentPacket ();
	} else {
		setRetry (m_currentTxPacket);
		m_dcf->notifyAccessOngoingError ();
	}
	m_dcf->notifyAccessFinished ();
	
}
void 
EdcaTxop::startNext (void)
{
	TRACE ("start next packet -- burst");
	m_firstPacketInBurst = false;
	tryToSendOnePacket ();
}
void
EdcaTxop::cancel (void)
{
	TRACE ("cancel tx");
	m_dcf->notifyAccessFinished ();
}

void
EdcaTxop::dropCurrentPacket (void)
{
	Packet::free (m_currentTxPacket);
	m_currentTxPacket = 0;
}
MacStation *
EdcaTxop::lookupDestStation (Packet *packet)
{
	return m_container->stations ()->lookup (getDestination (packet));
}
