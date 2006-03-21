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

#include "dca-txop.h"
#include "dcf.h"
#include "mac-parameters.h"
#include "mac-low.h"
#include "mac-queue-80211e.h"
#include "mac-tx-middle.h"
#include "packet.h"


#define noDCA_TXOP_TRACE 1

#ifdef DCA_TXOP_TRACE
#include "simulator.h"
#include <iostream>
# define TRACE(x) \
  std::cout <<"DCA TXOP now="<<Simulator::now_us ()<<"us "<<x<<std::endl;
#else /* DCA_TXOP_TRACE */
# define TRACE(x)
#endif /* DCA_TXOP_TRACE */

namespace yans {

class DcaTxop::AccessListener : public DcfAccessListener {
public:
	AccessListener (DcaTxop *txop)
		: DcfAccessListener (),
		  m_txop (txop) {}

	virtual ~AccessListener () {}

	virtual void access_granted_now (void)
	{
		m_txop->access_granted_now ();
	}
	virtual bool access_needed (void)
	{
		return m_txop->access_needed ();
	}
	virtual bool accessing_and_will_notify (void)
	{
		return m_txop->accessing_and_will_notify ();
	}

private:
	DcaTxop *m_txop;
};

class DcaTxop::TransmissionListener : public MacLowTransmissionListener {
public:
	TransmissionListener (DcaTxop *txop)
		: MacLowTransmissionListener (),
		  m_txop (txop) {}
		  
	virtual ~TransmissionListener () {}

	virtual void got_cts (double snr, uint8_t txMode) {
		m_txop->got_cts (snr, txMode);
	}
	virtual void missed_cts (void) {
		m_txop->missed_cts ();
	}
	virtual void got_ack (double snr, uint8_t txMode) {
		m_txop->got_ack (snr, txMode);
	}
	virtual void missed_ack (void) {
		m_txop->missed_ack ();
	}
	virtual void start_next (void) {
		m_txop->start_next ();
	}

	virtual void cancel (void) {
		assert (false);
	}

private:
	DcaTxop *m_txop;
};

DcaTxop::DcaTxop ()
	: m_access_listener (0),
	  m_current_packet (0),
	  m_ssrc (0),
	  m_slrc (0)
{
	m_transmission_listener = new DcaTxop::TransmissionListener (this);
}

DcaTxop::~DcaTxop ()
{
	delete m_access_listener;
	delete m_transmission_listener;
}

void 
DcaTxop::set_dcf (Dcf *dcf)
{
	m_dcf = dcf;
	m_access_listener = new DcaTxop::AccessListener (this);
	m_dcf->register_access_listener (m_access_listener);
}
void 
DcaTxop::set_queue (MacQueue80211e *queue)
{
	m_queue = queue;
}
void 
DcaTxop::set_low (MacLow *low)
{
	m_low = low;
}
void 
DcaTxop::set_parameters (MacParameters *parameters)
{
	m_parameters = parameters;
}
void 
DcaTxop::set_tx_middle (MacTxMiddle *tx_middle)
{
	m_tx_middle = tx_middle;
}
void 
DcaTxop::set_ack_received_callback (AckReceived *callback)
{
	m_ack_received = callback;
}

MacLow *
DcaTxop::low (void)
{
	return m_low;
}

MacParameters *
DcaTxop::parameters (void)
{
	return m_parameters;
}


bool
DcaTxop::need_rts (void)
{
	if (m_current_packet->get_size () > parameters ()->get_rts_cts_threshold ()) {
		return true;
	} else {
		return false;
	}
}

bool
DcaTxop::need_fragmentation (void)
{
	if (m_current_packet->get_size () > parameters ()->get_fragmentation_threshold ()) {
		return true;
	} else {
		return false;
	}
}

uint32_t
DcaTxop::get_n_fragments (void)
{
	uint32_t n_fragments = m_current_packet->get_size () / parameters ()->get_fragmentation_threshold () + 1;
	return n_fragments;
}
void
DcaTxop::next_fragment (void)
{
	m_fragment_number++;
}

uint32_t
DcaTxop::get_last_fragment_size (void)
{
	uint32_t last_fragment_size = m_current_packet->get_size () %
		parameters ()->get_fragmentation_threshold ();
	return last_fragment_size;
}

uint32_t
DcaTxop::get_fragment_size (void)
{
	return parameters ()->get_fragmentation_threshold ();
}
bool
DcaTxop::is_last_fragment (void) 
{
	if (m_fragment_number == (get_n_fragments () - 1)) {
		return true;
	} else {
		return false;
	}
}

uint32_t
DcaTxop::get_next_fragment_size (void) 
{
	if (is_last_fragment ()) {
		return 0;
	}
	
	uint32_t next_fragment_number = m_fragment_number + 1;
	if (next_fragment_number == (get_n_fragments () - 1)) {
		return get_last_fragment_size ();
	} else {
		return get_fragment_size ();
	}
}

Packet *
DcaTxop::get_fragment_packet (ChunkMac80211Hdr *hdr)
{
	*hdr = m_current_hdr;
	hdr->set_fragment_number (m_fragment_number);
	uint32_t start_offset = m_fragment_number * get_fragment_size ();
	Packet *fragment;
	if (is_last_fragment ()) {
		hdr->set_no_more_fragments ();
		fragment = m_current_packet->copy (start_offset, get_last_fragment_size ());
	} else {
		hdr->set_more_fragments ();
		fragment = m_current_packet->copy (start_offset, get_fragment_size ());
	}
	return fragment;
}

bool 
DcaTxop::accessing_and_will_notify (void)
{
	if (m_current_packet != 0) {
		return true;
	} else {
		return false;
	}
}

bool 
DcaTxop::access_needed (void)
{
	if (!m_queue->is_empty () ||
	    m_current_packet != 0) {
		TRACE ("access needed here");
		return true;
	} else {
		TRACE ("no access needed here");
		return false;
	}
}

void
DcaTxop::access_granted_now (void)
{
	if (!m_current_packet) {
		if (m_queue->is_empty ()) {
			TRACE ("queue empty");
			return;
		}
		m_current_packet = m_queue->dequeue (&m_current_hdr);
		assert (m_current_packet != 0);
		uint16_t sequence = m_tx_middle->get_next_sequence_number_for (&m_current_hdr);
		m_current_hdr.set_sequence_number (sequence);
		m_current_hdr.set_fragment_number (0);
		m_current_hdr.set_no_more_fragments ();
		m_ssrc = 0;
		m_slrc = 0;
		m_fragment_number = 0;
		TRACE ("dequeued size="<<m_current_packet->get_size ()<<
		       ", to="<<m_current_hdr.get_addr1 ()<<
		       ", seq="<<m_current_hdr.get_sequence_control ()); 
	}
	MacLowTransmissionParameters params;
	params.disable_override_duration_id ();
	if (m_current_hdr.get_addr1 ().is_broadcast ()) {
		params.disable_rts ();
		params.disable_ack ();
		params.disable_next_data ();
		low ()->start_transmission (m_current_packet,
					    &m_current_hdr,
					    params,
					    m_transmission_listener);
		m_current_packet->unref ();
		m_current_packet = 0;
		m_dcf->notify_access_ongoing_ok ();
		m_dcf->notify_access_finished ();
		TRACE ("tx broadcast");
	} else {
		params.enable_ack ();

		if (need_fragmentation ()) {
			params.disable_rts ();
			ChunkMac80211Hdr hdr;
			Packet *fragment = get_fragment_packet (&hdr);
			if (is_last_fragment ()) {
				TRACE ("fragmenting last fragment size="<<fragment->get_size ());
				params.disable_next_data ();
			} else {
				TRACE ("fragmenting size="<<fragment->get_size ());
				params.enable_next_data (get_next_fragment_size ());
			}
			low ()->start_transmission (fragment, &hdr, params, 
						    m_transmission_listener);
			fragment->unref ();
		} else {
			if (need_rts ()) {
				params.enable_rts ();
				TRACE ("tx unicast rts");
			} else {
				params.disable_rts ();
				TRACE ("tx unicast");
			}
			params.disable_next_data ();
			low ()->start_transmission (m_current_packet, &m_current_hdr,
						    params, m_transmission_listener);
		}
	}
}


void 
DcaTxop::got_cts (double snr, uint8_t txMode)
{
	TRACE ("got cts");
	m_ssrc = 0;
}
void 
DcaTxop::missed_cts (void)
{
	TRACE ("missed cts");
	m_ssrc++;
	if (m_ssrc > parameters ()->get_max_ssrc ()) {
		// to reset the dcf.
		m_dcf->notify_access_ongoing_error_but_ok ();
		m_dcf->notify_access_finished ();
		m_current_packet->unref ();
		m_current_packet = 0;
	} else {
		m_dcf->notify_access_ongoing_error ();
		m_dcf->notify_access_finished ();
	}
}
void 
DcaTxop::got_ack (double snr, uint8_t txMode)
{
	TRACE ("got ack");
	m_slrc = 0;
	if (!need_fragmentation () ||
	    is_last_fragment ()) {
		(*m_ack_received) (m_current_hdr);

		/* we are not fragmenting or we are done fragmenting
		 * so we can get rid of that packet now.
		 */
		m_current_packet->unref ();
		m_current_packet = 0;
		m_dcf->notify_access_ongoing_ok ();
		m_dcf->notify_access_finished ();
	}
}
void 
DcaTxop::missed_ack (void)
{
	TRACE ("missed ack");
	m_slrc++;
	if (m_slrc > parameters ()->get_max_slrc ()) {
		// to reset the dcf.		
		m_dcf->notify_access_ongoing_error_but_ok ();
		m_dcf->notify_access_finished ();
		m_current_packet->unref ();
		m_current_packet = 0;
	} else {
		// XXX
		//setRetry (m_currentTxPacket); 
		m_dcf->notify_access_ongoing_error ();
		m_dcf->notify_access_finished ();
	}
	
}
void 
DcaTxop::start_next (void)
{
	TRACE ("start next packet fragment");
	/* this callback is used only for fragments. */
	next_fragment ();
	ChunkMac80211Hdr hdr;
	Packet *fragment = get_fragment_packet (&hdr);
	MacLowTransmissionParameters params;
	params.enable_ack ();
	params.disable_rts ();
	params.disable_override_duration_id ();
	if (is_last_fragment ()) {
		params.disable_next_data ();
	} else {
		params.enable_next_data (get_next_fragment_size ());
	}
	low ()->start_transmission (fragment, &hdr, params, m_transmission_listener);
}

}; // namespace yans
