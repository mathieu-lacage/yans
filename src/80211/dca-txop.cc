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
#include "dca-txop.h"
#include "dcf.h"
#include "mac-parameters.h"
#include "mac-low.h"
#include "mac-queue-80211e.h"
#include "mac-station.h"
#include "mac-stations.h"
#include "mac-high.h"
#include "mac-container.h"


#define nopeDCA_TXOP_TRACE 1

#ifdef DCA_TXOP_TRACE
# define TRACE(format, ...) \
  printf ("DCA TXOP %d " format "\n", m_container->selfAddress (), ## __VA_ARGS__);
#else /* DCF_TRACE */
# define TRACE(format, ...)
#endif /* DCF_TRACE */


class MyAccessListener : public DcfAccessListener {
public:
	MyAccessListener (DcaTxop *txop)
		: DcfAccessListener (),
		  m_txop (txop) {}

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
	DcaTxop *m_txop;
};

class MyTransmissionListener : public MacLowTransmissionListener {
public:
	MyTransmissionListener (DcaTxop *txop)
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
	virtual void txCompletedAndSIFS (void) {
		m_txop->txCompletedAndSIFS ();
	}
	virtual void gotBlockAckStart (double snr) {
		m_txop->gotBlockAckStart (snr);
	}
	virtual void gotBlockAck (int sequence) {
		m_txop->gotBlockAck (sequence);
	}
	virtual void gotBlockAckEnd (void) {
		m_txop->gotBlockAckEnd ();
	}
	virtual void missedBlockAck (void) {
		m_txop->missedBlockAck ();
	}

private:
	DcaTxop *m_txop;
};



DcaTxop::DcaTxop (Dcf *dcf, MacQueue80211e *queue, MacContainer *container)
	: m_dcf (dcf),
	  m_queue (queue),
	  m_container (container),
	  m_currentTxPacket (0),
	  m_SSRC (0),
	  m_SLRC (0),
	  m_sequence (0)
{
	m_dcf->registerAccessListener (new MyAccessListener (this));
	m_transmissionListener = new MyTransmissionListener (this);
}

double
DcaTxop::now (void)
{
	return Scheduler::instance ().clock ();
}

MacParameters *
DcaTxop::parameters (void)
{
	return m_container->parameters ();
}

MacLow *
DcaTxop::low (void)
{
	return m_container->macLow ();
}

bool
DcaTxop::needRTS (void)
{
	if (getSize (m_currentTxPacket) > parameters ()->getRTSCTSThreshold ()) {
		return true;
	} else {
		return false;
	}
}

bool
DcaTxop::needFragmentation (void)
{
	if (getSize (m_currentTxPacket) > parameters ()->getFragmentationThreshold ()) {
		return true;
	} else {
		return false;
	}
}

int
DcaTxop::getNFragments (void)
{
	int nFragments = getSize (m_currentTxPacket) / parameters ()->getFragmentationThreshold () + 1;
	return nFragments;
}
void
DcaTxop::nextFragment (void)
{
	m_fragmentNumber++;
}

int
DcaTxop::getLastFragmentSize (void)
{
	int lastFragmentSize = getSize (m_currentTxPacket) % parameters ()->getFragmentationThreshold ();
	return lastFragmentSize;
}

int
DcaTxop::getFragmentSize (void)
{
	return parameters ()->getFragmentationThreshold ();
}
bool
DcaTxop::isLastFragment (void) 
{
	if (m_fragmentNumber == (getNFragments () - 1)) {
		return true;
	} else {
		return false;
	}
}

Packet *
DcaTxop::getFragmentPacket (void)
{
	Packet *fragment = m_currentTxPacket->copy ();
	if (isLastFragment ()) {
		setMoreFragments (fragment, false);
		setSize (fragment, getLastFragmentSize ());
	} else {
		setMoreFragments (fragment, true);
		setSize (fragment, getFragmentSize ());
	}
	setFragmentNumber (fragment, m_fragmentNumber);
	return fragment;
}

bool 
DcaTxop::accessingAndWillNotify (void)
{
	if (m_currentTxPacket != 0) {
		return true;
	} else {
		return false;
	}
}

bool 
DcaTxop::accessNeeded (void)
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

void
DcaTxop::accessGrantedNow (void)
{
	if (!m_currentTxPacket) {
		if (m_queue->isEmpty ()) {
			TRACE ("queue empty");
			return;
		}
		m_currentTxPacket = m_queue->dequeue ();
		initialize (m_currentTxPacket);
		assert (m_currentTxPacket != 0);
		m_sequence++;
		m_sequence %= 4096;
		setSequenceNumber (m_currentTxPacket, m_sequence);
		m_SSRC = 0;
		m_SLRC = 0;
		m_fragmentNumber = 0;
		TRACE ("dequeued %d to %d", getSize (m_currentTxPacket), getDestination (m_currentTxPacket));
	}
	if (getDestination (m_currentTxPacket) == (int)MAC_BROADCAST) {
		low ()->disableRTS ();
		low ()->disableACK ();
		low ()->setDataTransmissionMode (0);
		low ()->setTransmissionListener (m_transmissionListener);
		low ()->disableWaitForSIFS ();
		low ()->startTransmission (m_currentTxPacket);
		m_currentTxPacket = 0;
		m_dcf->notifyAccessOk ();
		TRACE ("tx broadcast");
	} else {
		int dataTxMode = lookupDestStation (m_currentTxPacket)->getDataMode (getSize (m_currentTxPacket));
		low ()->setDataTransmissionMode (dataTxMode);
		low ()->enableACK ();
		low ()->setTransmissionListener (m_transmissionListener);
		if (needFragmentation ()) {
			low ()->disableRTS ();
			Packet *fragment = getFragmentPacket ();
			if (isLastFragment ()) {
				TRACE ("fragmenting last fragment %d", getSize (fragment));
				low ()->disableWaitForSIFS ();
			} else {
				TRACE ("fragmenting %d", getSize (fragment));
				low ()->enableWaitForSIFS ();
			}
			// XXX: we are missing correct duration calculation
			// we need to add the txtime for the next fragment here.
			low ()->startTransmission (fragment);
		} else {
			if (needRTS ()) {
				low ()->enableRTS ();
				int txMode = lookupDestStation (m_currentTxPacket)->getRTSMode ();
				low ()->setRtsTransmissionMode (txMode);
				TRACE ("tx unicast rts mode %d/%d", txMode, dataTxMode);
			} else {
				low ()->disableRTS ();
				TRACE ("tx unicast mode %d", dataTxMode);
			}
			low ()->disableWaitForSIFS ();
			low ()->startTransmission (m_currentTxPacket->copy ());
		}
	}
}


void 
DcaTxop::gotCTS (double snr, int txMode)
{
	TRACE ("got cts");
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportRTSOk (snr, txMode);
	m_SSRC = 0;
}
void 
DcaTxop::missedCTS (void)
{
	TRACE ("missed cts at %f", now ());
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportRTSFailed ();
	m_SSRC++;
	if (m_SSRC > parameters ()->getMaxSSRC ()) {
		station->reportFinalRTSFailed ();
		// to reset the dcf.
		m_dcf->notifyAccessOk ();
		dropCurrentPacket ();
	} else {
		m_dcf->notifyAccessFailed ();
		m_dcf->requestAccess ();
	}
}
void 
DcaTxop::gotACK (double snr, int txMode)
{
	TRACE ("got ack");
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataOk (snr, txMode);
	m_SLRC = 0;
	if (!needFragmentation () ||
	    isLastFragment ()) {
		/* we are not fragmenting or we are done fragmenting
		 * so we can get rid of that packet now.
		 */
		Packet::free (m_currentTxPacket);
		m_currentTxPacket = 0;
		m_dcf->notifyAccessOk ();
	}
}
void 
DcaTxop::missedACK (void)
{
	TRACE ("missed ack");
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataFailed ();
	m_SLRC++;
	if (m_SLRC > parameters ()->getMaxSLRC ()) {
		station->reportFinalDataFailed ();
		// to reset the dcf.
		m_dcf->notifyAccessOk ();
		dropCurrentPacket ();
	} else {
		m_dcf->notifyAccessFailed ();
		m_dcf->requestAccess ();
	}
	
}
void 
DcaTxop::txCompletedAndSIFS (void)
{
	TRACE ("SIFS after tx complete");
	/* this callback is used only for fragments. */
	MacStation *station = lookupDestStation (m_currentTxPacket);
	low ()->disableRTS ();
	nextFragment ();
	if (isLastFragment ()) {
		low ()->disableWaitForSIFS ();
	} else {
		low ()->enableWaitForSIFS ();
	}
	Packet *fragment = getFragmentPacket ();
	low ()->setDataTransmissionMode (station->getDataMode (getSize (fragment)));
	low ()->startTransmission (fragment);
}
void 
DcaTxop::gotBlockAckStart (double snr)
{}
void 
DcaTxop::gotBlockAck (int sequence)
{}
void 
DcaTxop::gotBlockAckEnd (void)
{}
void 
DcaTxop::missedBlockAck (void)
{}


void
DcaTxop::dropCurrentPacket (void)
{
	Packet::free (m_currentTxPacket);
	m_currentTxPacket = 0;
}
MacStation *
DcaTxop::lookupDestStation (Packet *packet)
{
	return m_container->stations ()->lookup (getDestination (packet));
}
