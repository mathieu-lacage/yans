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
#include "yans/packet.h"
#include "phy-80211.h"
#include "yans/trace-container.h"

#include <cassert>


#define noDCA_TXOP_TRACE 1

#ifdef DCA_TXOP_TRACE
#include "yans/simulator.h"
#include <iostream>
# define TRACE(x) \
  std::cout <<"DCA TXOP now="<<Simulator::now_us ()<<"us "<<x<<std::endl;
#else /* DCA_TXOP_TRACE */
# define TRACE(x)
#endif /* DCA_TXOP_TRACE */

namespace yans {

class DcaTxop::NavListener : public yans::MacLowNavListener {
public:
	NavListener (yans::Dcf *dcf)
		: m_dcf (dcf) {}
	virtual ~NavListener () {}
	virtual void nav_start_us (uint64_t now_us, uint64_t duration_us) {
		m_dcf->notify_nav_start (now_us, duration_us);
	}
	virtual void nav_continue_us (uint64_t now_us, uint64_t duration_us) {
		m_dcf->notify_nav_continue (now_us, duration_us);
	}
	virtual void nav_reset_us (uint64_t now_us, uint64_t duration_us) {
		m_dcf->notify_nav_reset (now_us, duration_us);
	}
private:
	yans::Dcf *m_dcf;
};
class DcaTxop::PhyListener : public yans::Phy80211Listener {
public:
	PhyListener (yans::Dcf *dcf)
		: m_dcf (dcf) {}
	virtual ~PhyListener () {}
	virtual void notify_rx_start (uint64_t duration_us) {
		m_dcf->notify_rx_start_now (duration_us);
	}
	virtual void notify_rx_end_ok (void) {
		m_dcf->notify_rx_end_ok_now ();
	}
	virtual void notify_rx_end_error (void) {
		m_dcf->notify_rx_end_error_now ();
	}
	virtual void notify_tx_start (uint64_t duration_us) {
		m_dcf->notify_tx_start_now (duration_us);
	}
	virtual void notify_cca_busy_start (uint64_t duration_us) {
		m_dcf->notify_cca_busy_start_now (duration_us);
	}
private:
	yans::Dcf *m_dcf;
};


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
	  m_has_current (false),
	  m_ssrc (0),
	  m_slrc (0)
{
	m_transmission_listener = new DcaTxop::TransmissionListener (this);
	m_dcf = new Dcf ();
	m_access_listener = new DcaTxop::AccessListener (this);
	m_dcf->register_access_listener (m_access_listener);
	m_queue = new MacQueue80211e ();
}

DcaTxop::~DcaTxop ()
{
	delete m_access_listener;
	delete m_transmission_listener;
	delete m_nav_listener;
	delete m_phy_listener;
	delete m_queue;
}

void 
DcaTxop::set_low (MacLow *low)
{
	m_low = low;
	m_nav_listener = new DcaTxop::NavListener (m_dcf);
	m_low->register_nav_listener (m_nav_listener);
}
void
DcaTxop::set_phy (Phy80211 *phy)
{
	m_phy_listener = new DcaTxop::PhyListener (m_dcf);
	phy->register_listener (m_phy_listener);
}
void 
DcaTxop::set_parameters (MacParameters *parameters)
{
	m_parameters = parameters;
	m_dcf->set_parameters (parameters);
}
void 
DcaTxop::set_tx_middle (MacTxMiddle *tx_middle)
{
	m_tx_middle = tx_middle;
}
void 
DcaTxop::set_tx_ok_callback (TxOk callback)
{
	m_tx_ok_callback = callback;
}
void 
DcaTxop::set_tx_failed_callback (TxFailed callback)
{
	m_tx_failed_callback = callback;
}

void 
DcaTxop::set_difs_us (uint64_t difs_us)
{
	m_dcf->set_difs_us (difs_us);
}
void 
DcaTxop::set_eifs_us (uint64_t eifs_us)
{
	m_dcf->set_eifs_us (eifs_us);
}
void 
DcaTxop::set_cw_bounds (uint32_t min, uint32_t max)
{
	m_dcf->set_cw_bounds (min, max);
}
void 
DcaTxop::set_max_queue_size (uint32_t size)
{
	m_queue->set_max_size (size);
}
void 
DcaTxop::set_max_queue_delay_us (uint64_t us)
{
	m_queue->set_max_delay_us (us);
}

void 
DcaTxop::register_traces (TraceContainer *container)
{
	m_dcf->register_traces (container);
	container->register_callback ("80211-dca-acktimeout", &m_acktimeout_trace);
	container->register_callback ("80211-dca-ctstimeout", &m_ctstimeout_trace);
}

void 
DcaTxop::queue (Packet packet, ChunkMac80211Hdr const &hdr)
{
	m_queue->enqueue (packet, hdr);
	m_dcf->request_access ();
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
	if (m_current_packet.get_size () > parameters ()->get_rts_cts_threshold ()) {
		return true;
	} else {
		return false;
	}
}

bool
DcaTxop::need_fragmentation (void)
{
	if (m_current_packet.get_size () > parameters ()->get_fragmentation_threshold ()) {
		return true;
	} else {
		return false;
	}
}

uint32_t
DcaTxop::get_n_fragments (void)
{
	uint32_t n_fragments = m_current_packet.get_size () / parameters ()->get_fragmentation_threshold () + 1;
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
	uint32_t last_fragment_size = m_current_packet.get_size () %
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

Packet 
DcaTxop::get_fragment_packet (ChunkMac80211Hdr *hdr)
{
	*hdr = m_current_hdr;
	hdr->set_fragment_number (m_fragment_number);
	uint32_t start_offset = m_fragment_number * get_fragment_size ();
	Packet fragment;
	if (is_last_fragment ()) {
		hdr->set_no_more_fragments ();
		fragment = m_current_packet.create_fragment (start_offset, 
							     get_last_fragment_size ());
	} else {
		hdr->set_more_fragments ();
		fragment = m_current_packet.create_fragment (start_offset, 
							     get_fragment_size ());
	}
	return fragment;
}

bool 
DcaTxop::accessing_and_will_notify (void)
{
	if (m_has_current) {
		return true;
	} else {
		return false;
	}
}

bool 
DcaTxop::access_needed (void)
{
	if (!m_queue->is_empty () ||
	    m_has_current) {
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
	if (!m_has_current) {
		if (m_queue->is_empty ()) {
			TRACE ("queue empty");
			return;
		}
		bool found;
		m_current_packet = m_queue->dequeue (&m_current_hdr, &found);
		assert (found);
		m_has_current = true;
		assert (m_has_current);
		uint16_t sequence = m_tx_middle->get_next_sequence_number_for (&m_current_hdr);
		m_current_hdr.set_sequence_number (sequence);
		m_current_hdr.set_fragment_number (0);
		m_current_hdr.set_no_more_fragments ();
		m_ssrc = 0;
		m_slrc = 0;
		m_fragment_number = 0;
		TRACE ("dequeued size="<<m_current_packet.get_size ()<<
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
		m_has_current = false;
		m_dcf->reset_cw ();
		m_dcf->start_backoff ();
		TRACE ("tx broadcast");
	} else {
		params.enable_ack ();

		if (need_fragmentation ()) {
			params.disable_rts ();
			ChunkMac80211Hdr hdr;
			Packet fragment = get_fragment_packet (&hdr);
			if (is_last_fragment ()) {
				TRACE ("fragmenting last fragment size="<<fragment->get_size ());
				params.disable_next_data ();
			} else {
				TRACE ("fragmenting size="<<fragment->get_size ());
				params.enable_next_data (get_next_fragment_size ());
			}
			low ()->start_transmission (fragment, &hdr, params, 
						    m_transmission_listener);
		} else {
			if (need_rts ()) {
				params.enable_rts ();
				TRACE ("tx unicast rts");
			} else {
				params.disable_rts ();
				TRACE ("tx unicast");
			}
			params.disable_next_data ();
			// We need to make a copy in case we need to 
			// retransmit the packet: the MacLow modifies the input
			// Packet so, we would retransmit a modified packet
			// if we were not to make a copy.
			Packet copy = m_current_packet;
			low ()->start_transmission (copy, &m_current_hdr,
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
	m_ctstimeout_trace (m_ssrc);
	if (m_ssrc > parameters ()->get_max_ssrc ()) {
		// to reset the dcf.
		m_dcf->reset_cw ();
		m_dcf->start_backoff ();
		m_has_current = false;
	} else {
		m_dcf->update_failed_cw ();
		m_dcf->start_backoff ();
	}
}
void 
DcaTxop::got_ack (double snr, uint8_t txMode)
{
	m_slrc = 0;
	if (!need_fragmentation () ||
	    is_last_fragment ()) {
		TRACE ("got ack. tx done.");
		if (!m_tx_ok_callback.is_null ()) {
			m_tx_ok_callback (m_current_hdr);
		}

		/* we are not fragmenting or we are done fragmenting
		 * so we can get rid of that packet now.
		 */
		m_has_current = false;
		m_dcf->reset_cw ();
		m_dcf->start_backoff ();
	} else {
		TRACE ("got ack. tx not done, size="<<m_current_packet.get_size ());
	}
}
void 
DcaTxop::missed_ack (void)
{
	TRACE ("missed ack");
	m_slrc++;
	m_acktimeout_trace (m_slrc);
	if (m_slrc > parameters ()->get_max_slrc ()) {
		// to reset the dcf.		
		m_dcf->reset_cw ();
		m_dcf->start_backoff ();
		m_has_current = false;
	} else {
		// XXX
		//setRetry (m_currentTxPacket); 
		if (!m_tx_failed_callback.is_null ()) {
			m_tx_failed_callback (m_current_hdr);
		}
		m_dcf->update_failed_cw ();
		m_dcf->start_backoff ();
	}
	
}
void 
DcaTxop::start_next (void)
{
	TRACE ("start next packet fragment");
	/* this callback is used only for fragments. */
	next_fragment ();
	ChunkMac80211Hdr hdr;
	Packet fragment = get_fragment_packet (&hdr);
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
