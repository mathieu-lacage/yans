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

class MyTransmissionListener : public TransmissionListener {
public:
	MyTransmissionListener (DcaTxop *txop)
		: TransmissionListener (),
		  m_txop (txop) {}
		  
	virtual ~MyTransmissionListener () {}

	virtual void gotCTS (double snr) {
		m_txop->gotCTS (snr);
	}
	virtual void missedCTS (void) {
		m_txop->missedCTS ();
	}
	virtual void gotACK (double snr) {
		m_txop->gotACK (snr);
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



DcaTxop::DcaTxop (Dcf *dcf, MacQueue80211e *queue, MacLow *low, MacParameters *parameters)
	: m_dcf (dcf),
	  m_queue (queue),
	  m_low (low),
	  m_currentTxPacket (0),
	  m_parameters (parameters),
	  m_SSRC (0),
	  m_SLRC (0),
	  m_sequence (0)
{
	m_dcf->registerAccessListener (new MyAccessListener (this));
	m_transmissionListener = new MyTransmissionListener (this);
}

void
DcaTxop::accessGrantedNow (void)
{
	if (!m_currentTxPacket) {
		if (m_queue->isEmpty ()) {
			return;
		}
		m_currentTxPacket = m_queue->dequeue ();
	}
	if (getDestination (m_currentTxPacket) == (int)MAC_BROADCAST) {
		m_low->disableRTS ();
		m_low->disableACK ();
		//XXX
		m_low->setTransmissionMode (0);
	} else {
		if (getSize (m_currentTxPacket) > m_parameters->getRTSCTSThreshold ()) {
			m_low->enableRTS ();
			//XXX
			m_low->setTransmissionMode (0);
		} else {
			m_low->disableRTS ();
			//XXX
			m_low->setTransmissionMode (0);
		}
		m_low->enableACK ();
	}

	m_sequence++;
	m_sequence %= 4096;

	setSequence (m_currentTxPacket, m_sequence);

	m_low->setTransmissionListener (m_transmissionListener);
	m_low->startTransmission (m_currentTxPacket->copy ());
}


void 
DcaTxop::gotCTS (double snr)
{
	//XXX
	m_SSRC = 0;
	m_low->setTransmissionMode (0);
}
void 
DcaTxop::missedCTS (void)
{
	m_SSRC++;
	if (m_SSRC > m_parameters->getMaxSSRC ()) {
		// to reset the dcf.
		m_dcf->notifyAccessOk ();
		dropCurrentPacket ();
	} else {
		//XXX
		m_low->setTransmissionMode (0);

		m_dcf->notifyAccessFailed ();
		m_dcf->requestAccess ();
	}
}
void 
DcaTxop::gotACK (double snr)
{
	m_SLRC = 0;
	m_dcf->notifyAccessOk ();
}
void 
DcaTxop::missedACK (void)
{
	m_SLRC++;
	if (m_SLRC > m_parameters->getMaxSLRC ()) {
		// to reset the dcf.
		m_dcf->notifyAccessOk ();
		dropCurrentPacket ();		
	} else {
		//XXX
		m_low->setTransmissionMode (0);

		m_dcf->notifyAccessFailed ();
		m_dcf->requestAccess ();
	}
	
}
void 
DcaTxop::txCompletedAndSIFS (void)
{}
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
