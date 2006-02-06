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

/* This is quite a simple HCCA-txop implementation. A more sophisticated
 * version should be able to support:
 *   - fragmentation
 *   - BlockAck
 *   - Direct Link Scheduling
 *   - fancy queue scheduling for any txop. Namely, you want to
 *     be able to schedule packets from other tsids than the
 *     one requested by the QAP.
 */

#include "hcca-txop.h"
#include "mac-queue-80211e.h"
#include "mac-low.h"
#include "mac-container.h"
#include "mac-stations.h"
#include "mac-station.h"

#ifndef HCCA_TXOP_TRACE
#define HCCA_TXOP_TRACE 1
#endif /* HCCA_TXOP_TRACE */

#ifdef HCCA_TXOP_TRACE
# define TRACE(format, ...) \
	printf ("HCCA TRACE %d %f " format "\n", m_container->selfAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* HCCA_TXOP_TRACE */
# define TRACE(format, ...)
#endif /* HCCA_TXOP_TRACE */


class MyTransmissionListener : public MacLowTransmissionListener {
public:
	MyTransmissionListener (HccaTxop *txop)
		: MacLowTransmissionListener (),
		  m_txop (txop) {}
		  
	virtual ~MyTransmissionListener () {}

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

private:
	HccaTxop *m_txop;
};


HccaTxop::HccaTxop (MacContainer *container)
	: m_container (container),
	  m_currentTxPacket (0)
{
	m_transmissionListener = new MyTransmissionListener (this);
}

MacLow *
HccaTxop::low (void) const
{
	return m_container->macLow ();
}
double
HccaTxop::now (void) const
{
	return Scheduler::instance ().clock ();
}
MacStation *
HccaTxop::lookupDestStation (Packet *packet) const
{
	MacStation *station = m_container->stations ()->lookup (getDestination (packet));
	return station;
}

bool
HccaTxop::enoughTimeFor (Packet *packet)
{
	low ()->disableRTS ();
	low ()->enableACK ();
	low ()->disableNextData ();
	
	MacStation *station = lookupDestStation (packet);
	low ()->setDataTransmissionMode (station->getDataMode (getSize (packet)));
	low ()->setData (packet);
	double duration = low ()->calculateHandshakeDuration ();
	low ()->clearData ();
	return enoughTimeFor (duration);
}
void
HccaTxop::setCurrentTsid (uint8_t tsid)
{
	m_currentTsid = tsid;
}
uint8_t
HccaTxop::getCurrentTsid (void)
{
	return m_currentTsid;
}
bool
HccaTxop::txCurrent (void)
{
	MacQueue80211e *queue = m_tsQueues[getCurrentTsid ()];
	assert (queue != 0);

	if (m_currentTxPacket == 0) {
		if (queue->isEmpty ()) {
			TRACE ("queue empty");
			return false;
		}
		if (!enoughTimeFor (queue->peekNextPacket ())) {
			TRACE ("not enough time to complete next packet");
			goto tryToSendQosNull;
		}
		assert (m_currentTxPacket == 0);
		m_currentTxPacket = queue->dequeue ();
	}
	if (!enoughTimeFor (m_currentTxPacket)) {
		/* we don't have enough time to transmit the
		 * current frame.
		 * XXX shouldn't we ask for a longer txop with
		 * some special frame ?
		 */
		TRACE ("not enough time for current packet.");
		goto tryToSendQosNull;
	}

	MacStation *station = lookupDestStation (m_currentTxPacket);
	if (!queue->isEmpty ()) {
		// burst for next packet ?
		Packet *nextPacket = queue->peekNextPacket ();
		low ()->enableNextData (getSize (nextPacket));
		TRACE ("send to %d burst next", getDestination (m_currentTxPacket));
	} else {
		low ()->disableNextData ();
		TRACE ("send to %d", getDestination (m_currentTxPacket));
	}
	low ()->disableRTS ();
	low ()->enableACK ();
	low ()->enableOverrideDurationId (getDurationIdLeft ());
	low ()->setDataTransmissionMode (station->getDataMode (getSize (m_currentTxPacket)));
	low ()->setData (m_currentTxPacket);
	low ()->setTransmissionListener (m_transmissionListener);
	low ()->startTransmission ();
	return true;
 tryToSendQosNull:
	
	return false;
}

void 
HccaTxop::tsAccessGranted (uint8_t tsid, double txopLimit)
{
	TRACE ("access granted for tsid %d for %f seconds", tsid, txopLimit);
	startTxop (txopLimit);
	setCurrentTsid (tsid);
	txCurrent ();
}

void
HccaTxop::startTxop (double txopLimit)
{
	m_txopLimit = txopLimit;
	m_txopStart = now ();
}

double
HccaTxop::getDurationIdLeft (void)
{
	return now () - m_txopStart + m_txopLimit;
}

bool
HccaTxop::enoughTimeFor (double duration)
{
	double end = now () + duration;
	double txopEnd = m_txopStart + m_txopLimit;
	if (end > txopEnd) {
		return false;
	} else {
		return true;
	}
}

bool 
HccaTxop::isTxopFinished (void)
{
	double left = m_txopLimit - (now () - m_txopStart);
	if (left > 0.0) {
		return false;
	} else {
		return true;
	}
}

void 
HccaTxop::acAccessGranted (enum ac_e ac, double txopLimit)
{
}

void 
HccaTxop::addStream (MacQueue80211e *queue, uint8_t TSID)
{
	assert (m_tsQueues[TSID] == 0);
	m_tsQueues[TSID] = queue;
}

void 
HccaTxop::deleteStream (uint8_t TSID)
{
	delete m_tsQueues[TSID];
	m_tsQueues[TSID] = 0;
}

void 
HccaTxop::addAccessCategory (MacQueue80211e *queue, enum ac_e ac)
{
	assert (m_acQueues[ac] == 0);
	m_acQueues[ac] = queue;
}

void 
HccaTxop::deleteAccessCategory (enum ac_e ac)
{
	delete m_acQueues[ac];
	m_acQueues[ac] = 0;
}

void
HccaTxop::dropCurrentPacket (void)
{
	Packet::free (m_currentTxPacket);
	m_currentTxPacket = 0;
}

void 
HccaTxop::gotCTS (double snr, int txMode)
{
	assert (false);
}
void 
HccaTxop::missedCTS (void)
{
	assert (false);
}
void 
HccaTxop::gotACK (double snr, int txMode)
{
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataOk (snr, txMode);
	m_SLRC = 0;
	dropCurrentPacket ();
}
void 
HccaTxop::missedACK (void)
{
	m_SLRC++;
	if (m_SLRC > 1) {
		/* For HCCA, I consider that if the first
		 * retransmission does not work, we have a big problem
		 * so we stop our txop.
		 */
		dropCurrentPacket ();
		// XXX should we attempt to tx a CF-END ?
	} else {
		setRetry (m_currentTxPacket);		
	}
}
void 
HccaTxop::startNext (void)
{
	txCurrent ();
}
