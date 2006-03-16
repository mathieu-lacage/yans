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
#include "mac-traces.h"
#include "mac-tx-middle.h"
#include "net-interface-80211.h"
#include "common.h"


#ifndef DCA_TXOP_TRACE
#define nopeDCA_TXOP_TRACE 1
#endif /* DCA_TXOP_TRACE */

#ifdef DCA_TXOP_TRACE
# define TRACE(format, ...) \
  printf ("DCA TXOP %d " format "\n", m_interface->get_mac_address (), ## __VA_ARGS__);
#else /* DCA_TXOP_TRACE */
# define TRACE(format, ...)
#endif /* DCA_TXOP_TRACE */


class MyDcaAccessListener : public DcfAccessListener {
public:
	MyDcaAccessListener (DcaTxop *txop)
		: DcfAccessListener (),
		  m_txop (txop) {}

	virtual ~MyDcaAccessListener () {}

	virtual void access_granted_now (void)
	{
		m_txop->accessGrantedNow ();
	}
	virtual bool access_needed (void)
	{
		return m_txop->accessNeeded ();
	}
	virtual bool accessing_and_will_notify (void)
	{
		return m_txop->accessingAndWillNotify ();
	}

private:
	DcaTxop *m_txop;
};

class MyDcaTransmissionListener : public MacLowTransmissionListener {
public:
	MyDcaTransmissionListener (DcaTxop *txop)
		: MacLowTransmissionListener (),
		  m_txop (txop) {}
		  
	virtual ~MyDcaTransmissionListener () {}

	virtual void got_cts (double snr, int txMode) {
		m_txop->got_cts (snr, txMode);
	}
	virtual void missed_cts (void) {
		m_txop->missedCTS ();
	}
	virtual void got_ack (double snr, int txMode) {
		m_txop->got_ack (snr, txMode);
	}
	virtual void missed_ack (void) {
		m_txop->missedACK ();
	}
	virtual void start_next (void) {
		m_txop->startNext ();
	}

	virtual void cancel (void) {
		assert (false);
	}

private:
	DcaTxop *m_txop;
};


DcaTxop::_dca_txop (Dcf *dcf, MacQueue80211e *queue)
	: m_dcf (dcf),
	  m_queue (queue),
	  m_current_tx_packet (0),
	  m__ssrc (0),
	  m__slrc (0)
{
	m_dcf->register_access_listener (new MyDcaAccessListener (this));
	m_transmission_listener = new MyDcaTransmissionListener (this);
}

void
DcaTxop::set_interface (NetInterface80211 *interface)
{
	m_interface = interface;
}

MacLow *
DcaTxop::low (void)
{
	return m_interface->low ();
}

MacParameters *
DcaTxop::parameters (void)
{
	return m_interface->parameters ();
}

double
DcaTxop::now (void)
{
	return Scheduler::instance ().clock ();
}

bool
DcaTxop::need_rts (void)
{
	if (getSize (m_currentTxPacket) > parameters ()->get_rtsctsthreshold ()) {
		return true;
	} else {
		return false;
	}
}

bool
DcaTxop::need_fragmentation (void)
{
	if (getSize (m_currentTxPacket) > parameters ()->get_fragmentation_threshold ()) {
		return true;
	} else {
		return false;
	}
}

int
DcaTxop::get_nfragments (void)
{
	int nFragments = getSize (m_currentTxPacket) / parameters ()->getFragmentationThreshold () + 1;
	return nFragments;
}
void
DcaTxop::next_fragment (void)
{
	m_fragment_number++;
}

int
DcaTxop::get_last_fragment_size (void)
{
	int lastFragmentSize = getSize (m_currentTxPacket) % parameters ()->getFragmentationThreshold ();
	return lastFragmentSize;
}

int
DcaTxop::get_fragment_size (void)
{
	return parameters ()->getFragmentationThreshold ();
}
bool
DcaTxop::is_last_fragment (void) 
{
	if (m_fragment_number == (getNFragments () - 1)) {
		return true;
	} else {
		return false;
	}
}

int
DcaTxop::get_next_fragment_size (void) 
{
	if (isLastFragment ()) {
		return 0;
	}
	
	int nextFragmentNumber = m_fragment_number + 1;
	if (nextFragmentNumber == (getNFragments () - 1)) {
		return getLastFragmentSize ();
	} else {
		return getFragmentSize ();
	}
}

Packet *
DcaTxop::get_fragment_packet (void)
{
	Packet *fragment = m_current_tx_packet->copy ();
	if (isLastFragment ()) {
		setMoreFragments (fragment, false);
		setSize (fragment, get_last_fragment_size ());
	} else {
		setMoreFragments (fragment, true);
		setSize (fragment, get_fragment_size ());
	}
	setFragmentNumber (fragment, m_fragmentNumber);
	return fragment;
}

bool 
DcaTxop::accessing_and_will_notify (void)
{
	if (m_currentTxPacket != 0) {
		return true;
	} else {
		return false;
	}
}

bool 
DcaTxop::access_needed (void)
{
	if (!m_queue->isEmpty () ||
	    m_currentTxPacket != 0) {
		TRACE ("access needed here");
		return true;
	} else {
		TRACE ("no access needed here -- %d/%p", m_queue->is_empty (), m_currentTxPacket);
		return false;
	}
}

void
DcaTxop::access_granted_now (void)
{
	if (!m_currentTxPacket) {
		if (m_queue->is_empty ()) {
			TRACE ("queue empty");
			return;
		}
		m_currentTxPacket = m_queue->dequeue ();
		initialize (m_currentTxPacket);
		assert (m_current_tx_packet != 0);
		uint16_t sequence = m_interface->txMiddle ()->get_next_sequence_number_for (m_currentTxPacket);
		setSequenceNumber (m_currentTxPacket, sequence);
		m__ssrc = 0;
		m__slrc = 0;
		m_fragment_number = 0;
		TRACE ("dequeued %d to %d seq: 0x%x", 
		       get_size (m_currentTxPacket), 
		       get_destination (m_currentTxPacket),
		       get_sequence_control (m_currentTxPacket));
	}
	low ()->disableOverrideDurationId ();
	if (getDestination (m_currentTxPacket) == (int)MAC_BROADCAST) {
		low ()->disableRTS ();
		low ()->disableACK ();
		low ()->set_data_transmission_mode (0);
		low ()->set_transmission_listener (m_transmissionListener);
		low ()->disableNextData ();
		low ()->set_data (m_currentTxPacket);
		low ()->startTransmission ();
		m_current_tx_packet = 0;
		m_dcf->notifyAccessOngoingOk ();
		m_dcf->notifyAccessFinished ();
		TRACE ("tx broadcast");
	} else {
		int dataTxMode = lookupDestStation (m_currentTxPacket)->get_data_mode (getSize (m_currentTxPacket));
		low ()->set_data_transmission_mode (dataTxMode);
		low ()->enableACK ();
		low ()->set_transmission_listener (m_transmissionListener);
		if (needFragmentation ()) {
			low ()->disableRTS ();
			Packet *fragment = getFragmentPacket ();
			if (isLastFragment ()) {
				TRACE ("fragmenting last fragment %d", get_size (fragment));
				low ()->disableNextData ();
			} else {
				TRACE ("fragmenting %d", get_size (fragment));
				low ()->enable_next_data (getNextFragmentSize (), dataTxMode);
			}
			low ()->set_data (fragment);
			low ()->startTransmission ();
		} else {
			if (needRTS ()) {
				low ()->enableRTS ();
				int txMode = lookup_dest_station (m_currentTxPacket)->getRTSMode ();
				low ()->set_rts_transmission_mode (txMode);
				TRACE ("tx unicast rts mode %d/%d", txMode, dataTxMode);
			} else {
				low ()->disableRTS ();
				TRACE ("tx unicast mode %d", dataTxMode);
			}
			low ()->disableNextData ();
			low ()->setData (m_current_tx_packet->copy ());
			low ()->startTransmission ();
		}
	}
}


void 
DcaTxop::got_cts (double snr, int txMode)
{
	TRACE ("got cts");
	MacStation *station = lookup_dest_station (m_currentTxPacket);
	station->report_rtsok (snr, txMode);
	m__ssrc = 0;
}
void 
DcaTxop::missed_cts (void)
{
	TRACE ("missed cts at %f", now ());
	MacStation *station = lookup_dest_station (m_currentTxPacket);
	station->reportRTSFailed ();
	m__ssrc++;
	if (m__ssrc > parameters ()->get_max_ssrc ()) {
		station->reportFinalRTSFailed ();
		// to reset the dcf.
		m_dcf->notifyAccessOngoingErrorButOk ();
		m_dcf->notifyAccessFinished ();
		dropCurrentPacket ();
	} else {
		m_dcf->notifyAccessOngoingError ();
		m_dcf->notifyAccessFinished ();
	}
}
void 
DcaTxop::got_ack (double snr, int txMode)
{
	TRACE ("got ack");
	MacStation *station = lookup_dest_station (m_currentTxPacket);
	station->report_data_ok (snr, txMode);
	m__slrc = 0;
	if (!needFragmentation () ||
	    is_last_fragment ()) {
		m_interface->high ()->notify_ack_received_for (m_currentTxPacket);

		/* we are not fragmenting or we are done fragmenting
		 * so we can get rid of that packet now.
		 */
		Packet::free (m_currentTxPacket);
		m_current_tx_packet = 0;
		m_dcf->notifyAccessOngoingOk ();
		m_dcf->notifyAccessFinished ();
	}
}
void 
DcaTxop::missed_ack (void)
{
	TRACE ("missed ack");
	MacStation *station = lookup_dest_station (m_currentTxPacket);
	station->reportDataFailed ();
	m__slrc++;
	if (m__slrc > parameters ()->get_max_slrc ()) {
		station->reportFinalDataFailed ();
		// to reset the dcf.
		m_dcf->notifyAccessOngoingErrorButOk ();
		m_dcf->notifyAccessFinished ();
		dropCurrentPacket ();
	} else {
		setRetry (m_currentTxPacket);
		m_dcf->notifyAccessOngoingError ();
		m_dcf->notifyAccessFinished ();
	}
	
}
void 
DcaTxop::start_next (void)
{
	TRACE ("start next packet");
	/* this callback is used only for fragments. */
	MacStation *station = lookup_dest_station (m_currentTxPacket);
	low ()->disableRTS ();
	nextFragment ();
	Packet *fragment = getFragmentPacket ();
	int txMode = station->get_data_mode (getSize (fragment));
	if (isLastFragment ()) {
		low ()->disableNextData ();
	} else {
		low ()->enable_next_data (getNextFragmentSize (), txMode);
	}
	low ()->set_data_transmission_mode (txMode);
	low ()->set_data (fragment);
	low ()->startTransmission ();
}

void
DcaTxop::drop_current_packet (void)
{
	Packet::free (m_currentTxPacket);
	m_current_tx_packet = 0;
}
MacStation *
DcaTxop::lookup_dest_station (Packet *packet)
{
	return m_interface->stations ()->lookup (getDestination (packet));
}
