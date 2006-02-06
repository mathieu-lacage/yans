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
#include "mac-stations.h"
#include "mac-station.h"
#include "mac-tx-middle.h"
#include "mac-parameters.h"
#include "net-interface-80211.h"
#include "phy-80211.h"

#include "packet.h"

#ifndef HCCA_TXOP_TRACE
#define HCCA_TXOP_TRACE 1
#endif /* HCCA_TXOP_TRACE */

#ifdef HCCA_TXOP_TRACE
# define TRACE(format, ...) \
	printf ("HCCA TRACE %d %f " format "\n", m_interface->getMacAddress (), \
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

	virtual void cancel (void) {
		assert (false);
	}

private:
	HccaTxop *m_txop;
};

class MyQosNullTransmissionListener : public MacLowTransmissionListener {
public:
	MyQosNullTransmissionListener (HccaTxop *txop)
		: MacLowTransmissionListener (),
		  m_txop (txop) {}
		  
	virtual ~MyQosNullTransmissionListener () {}

	virtual void gotCTS (double snr, int txMode) {
		assert (false);
	}
	virtual void missedCTS (void) {
		assert (false);
	}
	virtual void gotACK (double snr, int txMode) {
		m_txop->gotQosNullAck ();
	}
	virtual void missedACK (void) {
		m_txop->missedQosNullAck ();
	}
	virtual void startNext (void) {
		assert (false);
	}
	virtual void cancel (void) {
		assert (false);
	}

private:
	HccaTxop *m_txop;
};


HccaTxop::HccaTxop ()
	: m_currentTxPacket (0)
{
	m_transmissionListener = new MyTransmissionListener (this);
	m_qosNullTransmissionListener = new MyQosNullTransmissionListener (this);
	m_txopStart = new StaticHandler<HccaTxop> (this, &HccaTxop::txopStartTimer);
}

void 
HccaTxop::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
}

MacLow *
HccaTxop::low (void)
{
	return m_interface->low ();
}

MacParameters *
HccaTxop::parameters (void)
{
	return m_interface->parameters ();
}

double
HccaTxop::now (void) const
{
	return Scheduler::instance ().clock ();
}
MacStation *
HccaTxop::lookupDestStation (Packet *packet) const
{
	MacStation *station = m_interface->stations ()->lookup (getDestination (packet));
	return station;
}

int
HccaTxop::getAckModeForDataMode (int destination, int txMode)
{
	/* XXX */
	return 0;
}

bool
HccaTxop::enoughTimeFor (Packet *packet)
{
	MacStation *station = lookupDestStation (packet);
	int txMode = station->getDataMode (getSize (packet));
	double duration = 0.0;
	duration += calculateTxDuration (txMode, getSize (packet));
	duration += parameters ()->getSIFS ();
	int ackTxMode = getAckModeForDataMode (getDestination (packet), txMode);
	duration += calculateTxDuration (ackTxMode, parameters ()->getPacketSize (MAC_80211_CTL_ACK));
	return enoughTimeFor (duration);
}
double 
HccaTxop::calculateTxDuration (int txMode, uint32_t size)
{
	return m_interface->phy ()->calculateTxDuration (txMode, size);
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
Packet *
HccaTxop::getQosNullFor (int destination)
{
	Packet *packet = hdr_mac_80211::create (m_interface->getMacAddress ());
	setDestination (packet, destination);
	setFinalDestination (packet, destination);
	setType (packet, MAC_80211_MGT_QOSNULL);
	setSize (packet, parameters ()->getPacketSize (MAC_80211_MGT_QOSNULL));
	return packet;
}
void
HccaTxop::tryToSendQosNull (void)
{
	Packet *qosNull = getQosNullFor (m_interface->getBSSID ());

	/* calculate whether or not we have enough time. at rate 0. */
	double duration = 0.0;
	duration += calculateTxDuration (0, getSize (qosNull));
	duration += parameters ()->getSIFS ();
	duration += calculateTxDuration (0, parameters ()->getPacketSize (MAC_80211_CTL_ACK));
	if (!enoughTimeFor (duration)) {
		Packet::free (qosNull);
		return;
	}
	low ()->disableNextData ();
	low ()->disableRTS ();
	low ()->enableSuperFastAck ();
	low ()->disableOverrideDurationId ();
	low ()->setDataTransmissionMode (0);
	low ()->setData (qosNull);
	low ()->setTransmissionListener (m_qosNullTransmissionListener);
	low ()->startTransmission ();	

}
bool
HccaTxop::txCurrent (void)
{
	MacQueue80211e *queue = m_tsQueues[getCurrentTsid ()];
	assert (queue != 0);

	if (m_currentTxPacket == 0) {
		if (queue->isEmpty ()) {
			TRACE ("queue empty");
			tryToSendQosNull ();
			return false;
		}
		if (!enoughTimeFor (queue->peekNextPacket ())) {
			TRACE ("not enough time to complete next packet");
			tryToSendQosNull ();
			return false;
		}
		assert (m_currentTxPacket == 0);
		m_currentTxPacket = queue->dequeue ();
		assert (m_currentTxPacket != 0);
		initialize (m_currentTxPacket);
		uint16_t sequence = m_interface->txMiddle ()->getNextSequenceNumberFor (m_currentTxPacket);
		setSequenceNumber (m_currentTxPacket, sequence);
		TRACE ("dequeued %d to %d seq: 0x%x", 
		       getSize (m_currentTxPacket), 
		       getDestination (m_currentTxPacket),
		       getSequenceControl (m_currentTxPacket));
	}
	if (!enoughTimeFor (m_currentTxPacket)) {
		/* we don't have enough time to transmit the
		 * current frame.
		 * XXX shouldn't we ask for a longer txop with
		 * some special frame ?
		 */
		TRACE ("not enough time for current packet.");
		tryToSendQosNull ();
		return false;
	}


	MacStation *station = lookupDestStation (m_currentTxPacket);
	int txMode = station->getDataMode (getSize (m_currentTxPacket));
	if (!queue->isEmpty ()) {
		// burst for next packet ?
		Packet *nextPacket = queue->peekNextPacket ();
		low ()->enableNextData (getSize (nextPacket), txMode);
		TRACE ("send to %d burst next", getDestination (m_currentTxPacket));
	} else {
		low ()->disableNextData ();
		TRACE ("send last to %d", getDestination (m_currentTxPacket));
	}
	low ()->disableRTS ();
	low ()->enableFastAck ();
	low ()->enableOverrideDurationId (getDurationIdLeft ());
	low ()->setDataTransmissionMode (txMode);
	low ()->setData (m_currentTxPacket->copy ());
	low ()->setTransmissionListener (m_transmissionListener);
	low ()->startTransmission ();
	return true;
}

void 
HccaTxop::tsAccessGranted (uint8_t tsid, double txopLimit)
{
	TRACE ("access granted for tsid %d for %f seconds", tsid, txopLimit);
	initializeTxop (txopLimit, tsid);
	m_txopStart->start (parameters ()->getSIFS ());
}


void
HccaTxop::txopStartTimer (void)
{
	TRACE ("starting txop");
	txCurrent ();
}
void
HccaTxop::initializeTxop (double txopLimit, int tsid)
{
	m_txopLimit = txopLimit;
	m_txopStartTime = now ();
	setCurrentTsid (tsid);
}

double
HccaTxop::getDurationIdLeft (void)
{
	return now () - m_txopStartTime + m_txopLimit;
}

bool
HccaTxop::enoughTimeFor (double duration)
{
	double end = now () + duration;
	double txopEnd = m_txopStartTime + m_txopLimit;
	if (end > txopEnd) {
		return false;
	} else {
		return true;
	}
}

bool 
HccaTxop::isTxopFinished (void)
{
	double left = m_txopLimit - (now () - m_txopStartTime);
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
HccaTxop::gotQosNullAck (void)
{
	TRACE ("got QosNull Ack  from %d", m_interface->getBSSID ());
	/* There is not much to do since this means that
	 * we have successfully transmitted ownership of the
	 * medium to the QAP.
	 */
}
void 
HccaTxop::missedQosNullAck (void)
{
	TRACE ("missed QosNull Ack from %d", m_interface->getBSSID ());
	/* We are not sure the QAP has correctly received the
	 * QosNull frame we sent to relinquish ownership of the medium
	 * so we try to trigger a retransmission of the QosNull now
	 * and until either it is received correctly or we don't have
	 * any time left in the txop.
	 */
	tryToSendQosNull ();
}
void 
HccaTxop::gotACK (double snr, int txMode)
{
	TRACE ("got ack from %d", getDestination (m_currentTxPacket));
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataOk (snr, txMode);
	m_SLRC = 0;
	dropCurrentPacket ();
}
void 
HccaTxop::missedACK (void)
{
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataFailed ();
	m_SLRC++;
	if (m_SLRC > 1) {
		/* For HCCA, I consider that if the first
		 * retransmission does not work, we have a big problem
		 * so we stop our txop.
		 */
		TRACE ("missed ack from %d -- dropping packet", getDestination (m_currentTxPacket));
		dropCurrentPacket ();
		// XXX should we attempt to tx a CF-END ?
	} else {
		TRACE ("missed ack from %d -- retry packet", getDestination (m_currentTxPacket));
		setRetry (m_currentTxPacket);
		txCurrent ();
	}
}
void 
HccaTxop::startNext (void)
{
	txCurrent ();
}
