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

class MyAccessListener : public DcfAccessListener {
public:
	MyAccessListener (DcaTxop *txop)
		: DcfAccessListener (),
		  m_txop (txop) {}

	virtual void accessGrantedNow (void)
	{
		m_txop->accessGrantedNow ();
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
	if (m_fragmentNumber == getNFragments ()) {
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
		setSize (fragment, getLastFragmentSize ());
	} else {
		setSize (fragment, getFragmentSize ());
	}
	setFragmentNumber (fragment, m_fragmentNumber);
	return fragment;
}

void
DcaTxop::accessGrantedNow (void)
{
	if (!m_currentTxPacket) {
		if (m_queue->isEmpty ()) {
			return;
		}
		m_currentTxPacket = m_queue->dequeue ();
		m_sequence++;
		m_sequence %= 4096;
		setSequenceNumber (m_currentTxPacket, m_sequence);
		m_SSRC = 0;
		m_SLRC = 0;
		m_fragmentNumber = 0;
	}
	if (getDestination (m_currentTxPacket) == (int)MAC_BROADCAST) {
		low ()->disableRTS ();
		low ()->disableACK ();
		low ()->setDataTransmissionMode (0);
	} else {
		if (needRTS ()) {
			low ()->enableRTS ();
			int txMode = lookupDestStation (m_currentTxPacket)->getRTSMode ();
			low ()->setRtsTransmissionMode (txMode);
		} else {
			low ()->disableRTS ();
			int txMode = lookupDestStation (m_currentTxPacket)->getDataMode (getSize (m_currentTxPacket));
			low ()->setDataTransmissionMode (txMode);
		}
		low ()->enableACK ();
	}

	low ()->setTransmissionListener (m_transmissionListener);

	if (needFragmentation ()) {
		if (isLastFragment ()) {
			low ()->disableWaitForSIFS ();
		} else {
			low ()->enableWaitForSIFS ();
		}
		Packet *fragment = getFragmentPacket ();
		// XXX: we are missing correct duration calculation
		// we need to add the txtime for the next fragment here.
		low ()->startTransmission (fragment);
	} else {
		low ()->disableWaitForSIFS ();
		low ()->startTransmission (m_currentTxPacket->copy ());
	}
}


void 
DcaTxop::gotCTS (double snr, int txMode)
{
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportRTSOk (snr, txMode);
	m_SSRC = 0;
}
void 
DcaTxop::missedCTS (void)
{
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
	MacStation *station = lookupDestStation (m_currentTxPacket);
	station->reportDataOk (snr, txMode);
	m_SLRC = 0;
	m_dcf->notifyAccessOk ();
}
void 
DcaTxop::missedACK (void)
{
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
	m_fragmentNumber++;
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
