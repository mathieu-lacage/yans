/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#include "mac-low.h"
#include "mac-high.h"
#include "phy-80211.h"
#include "mac-station.h"
#include "rng-uniform.h"
#include "mac-parameters.h"
#include "mac-traces.h"
#include "net-interface.h"
#include "mac-rx-middle.h"
#include "net-interface-80211.h"
#include "common.h"

#include "packet.h"

#include <iostream>

#ifndef MAC_LOW_TRACE
#define nopeMAC_LOW_TRACE 1
#endif

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
MacLowNavListener::MacLowNavListener ()
{}
MacLowNavListener::~MacLowNavListener ()
{}

MacLow::MacLow ()
	: m_normal_ack_timeout_event (0),
	  m_fast_ack_timeout_event (0),
	  m_super_fast_ack_timeout_event (0),
	  m_fast_ack_failed_timeout_event (0),
	  m_cts_timeout_event (0),
	  m_send_cts_event (0),
	  m_send_ack_event (0),
	  m_send_data_event (0),
	  m_wait_sifs_event (0),
	  m_currentTxPacket (0)
{
	m_lastNavDuration = 0.0;
	m_lastNavStart = 0.0;
	m_overrideDurationId = 0.0;
	m_currentTxPacket = 0;
}

MacLow::~MacLow ()
{
	if (m_normal_ack_timeout_event != 0) {
		m_normal_ack_timeout_event->cancel ();
		m_normal_ack_timeout_event = 0;
	}
	if (m_fast_ack_timeout_event != 0) {
		m_fast_ack_timeout_event->cancel ();
		m_fast_ack_timeout_event = 0;
	}
	if (m_super_fast_ack_timeout_event != 0) {
		m_super_fast_ack_timeout_event->cancel ();
		m_super_fast_ack_timeout_event = 0;
	}
	if (m_fast_ack_failed_timeout_event != 0) {
		m_fast_ack_failed_timeout_event->cancel ();
		m_fast_ack_failed_timeout_event = 0;
	}
	if (m_cts_timeout_event != 0) {
		m_cts_timeout_event->cancel ();
		m_cts_timeout_event= 0;
	}
	if (m_send_cts_event != 0) {
		m_send_cts_event->cancel ();
		m_send_cts_event= 0;
	}
	if (m_send_ack_event != 0) {
		m_send_ack_event->cancel ();
		m_send_ack_event= 0;
	}
	if (m_send_data_event != 0) {
		m_send_data_event->cancel ();
		m_send_data_event= 0;
	}
	if (m_wait_sifs_event != 0) {
		m_wait_sifs_event->cancel ();
		m_wait_sifs_event= 0;
	}
}

/*****************************************************
 *  Accessor methods
 ****************************************************/

void
MacLow::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
}

void
MacLow::forwardDown (Packet *packet, int tx_mode, ChunkMac80211Hdr *const hdr)
{
	double txDuration = calculateTxDuration (tx_mode, packet->get_size ());
	m_interface->phy ()->sendDown (packet);
	/* Note that it is really important to notify the NAV 
	 * thing _after_ forwarding the packet to the PHY.
	 * but we must perform the calculations _before_ passing
	 * the thing to the PHY (I think I remember the PHY does
	 * some nasty things to the underlying memory of the 
	 * packet)
	 */

	notifyNav (now ()+txDuration, hdr);
}
int 
MacLow::getSelf (void)
{
	return m_interface->getMacAddress ();
}
double
MacLow::calculateTxDuration (int mode, uint32_t size)
{
	double duration = m_interface->phy ()->calculateTxDuration (mode, size);
	return duration;
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

void
MacLow::maybeCancelPrevious (void)
{
	/* The cancels below should not be really needed in 
	 * a perfect world but the QapScheduler steals ownership
	 * of the medium to the edca-txop which cannot complete
	 * any transaction left.
	 * See the comment in setData too.
	 */
	if (m_normal_ack_timeout_event != 0) {
		m_normal_ack_timeout_event->cancel ();
		m_normal_ack_timeout_event = 0;
	}
	if (m_fast_ack_timeout_event != 0) {
		m_fast_ack_timeout_event->cancel ();
		m_fast_ack_timeout_event = 0;
	}
	if (m_super_fast_ack_timeout_event != 0) {
		m_super_fast_ack_timeout_event->cancel ();
		m_super_fast_ack_timeout_event = 0;
	}
	if (m_fast_ack_failed_timeout_event != 0) {
		m_fast_ack_failed_timeout_event->cancel ();
		m_fast_ack_failed_timeout_event = 0;
	}
	if (m_cts_timeout_event != 0) {
		m_cts_timeout_event->cancel ();
		m_cts_timeout_event= 0;
	}
	if (m_send_cts_event != 0) {
		m_send_cts_event->cancel ();
		m_send_cts_event= 0;
	}
	if (m_send_ack_event != 0) {
		m_send_ack_event->cancel ();
		m_send_ack_event= 0;
	}
	if (m_send_data_event != 0) {
		m_send_data_event->cancel ();
		m_send_data_event= 0;
	}
	if (m_wait_sifs_event != 0) {
		m_wait_sifs_event->cancel ();
		m_wait_sifs_event= 0;
	}

	if (m_transmissionListener != 0) {
		m_transmissionListener->cancel ();
	}
}

uint32_t 
MacLow::get_ack_size (void) const
{
	ChunkMac80211Hdr ack;
	ack.set_type (MAC_80211_CTL_ACK);
	return ack.get_size ();
}
uint32_t 
MacLow::get_rts_size (void) const
{
	ChunkMac80211Hdr rts;
	rts.set_type (MAC_80211_CTL_RTS);
	return rts.get_size ();
}
uint32_t 
MacLow::get_cts_size (void) const
{
	ChunkMac80211Hdr cts;
	cts.set_type (MAC_80211_CTL_CTS);
	return cts.get_size ();
}
double 
MacLow::get_sifs (void) const
{
	// XXX
	return 0.0;
}
double 
MacLow::get_pifs (void) const
{
	// XXX
	return 0.0;
}
double 
MacLow::get_ack_timeout (void) const
{
	// XXX
	return 0.0;
}
double 
MacLow::get_cts_timeout (void) const
{
	// XXX
	return 0.0;
}
uint32_t 
MacLow::get_current_size (void) const
{
	ChunkMac80211Fcs fcs;
	return m_current_packet.get_size () + m_current_hdr.get_size () + fcs.get_size ();
}

double
MacLow::now (void)
{
	double now;
	now = Simulator::now_s ();
	return now;
}

void
MacLow::send_rts_for_packet (void)
{
	/* send an RTS for this packet. */
	ChunkMac80211Hdr rts;
	rts.set_type (MAC_80211_CTL_RTS);
	rts.set_addr1 (m_current_hdr.get_addr1 ());
	rts.set_addr2 (getSelf ());
	double duration;
	if (m_overrideDurationId > 0.0) {
		duration = m_overrideDurationId;
	} else {
		int ackTxMode = getAckTxModeForData (m_current_hdr.get_addr1 (), m_dataTxMode);
		duration = 0.0;
		duration += m_interface->parameters ()->getSIFS ();
		duration += calculateTxDuration (m_rtsTxMode, get_cts_size ());
		duration += m_interface->parameters ()->getSIFS ();
		duration += calculateTxDuration (m_dataTxMode, m_current_packet->get_size () + m_current_hdr.get_size () + 4);
		duration += m_interface->parameters ()->getSIFS ();
		duration += calculateTxDuration (ackTxMode, get_ack_size ());
	}
	rts.set_duration_s (duration);

	TRACE ("tx RTS to "<< rts.get_addr1 () << ", mode=" << m_rtsTxMode);

	double txDuration = calculateTxDuration (m_rtsTxMode, get_rts_size ());
	double timerDelay = txDuration + m_interface->parameters ()->getCTSTimeoutDuration ();

	assert (m_cts_timeout_event == 0);
	m_cts_timeout_event = make_event (&MacLow::cts_timeout, this);
	Simulator::insert_in_s (timerDelay, m_cts_timeout_event);

	Packet *packet = new Packet ();
	packet->add (&rts);
	ChunkMac80211Fcs fcs;
	packet->add (&fcs);

	forward_down (packet, m_rtsTxMode, &rts);
}
void
MacLow::send_data_packet (void)
{
	/* send this packet directly. No RTS is needed. */
	TRACE ("tx "<< m_current_hdr.get_type_string () << " to " << m_current_hdr.get_addr1 () <<
	       ", mode=" << m_dataTxMode);
	double txDuration = calculateTxDuration (m_dataTxMode, m_current_packet.get_size () + m_current_hdr.get_size () + 4);
	if (waitNormalAck ()) {
		double timerDelay = txDuration + get_ack_timeout ();
		assert (m_normal_ack_timeout_event == 0);
		m_normal_ack_timeout_event = make_event (&MacLow::normal_ack_timeout, this);
		Simulator::insert_in_s (timerDelay, m_normal_ack_timeout_event);
	} else if (waitFastAck ()) {
		double timerDelay = txDuration + get_pifs ();
		assert (m_fast_ack_timeout_event == 0);
		m_fast_ack_timeout_event = make_event (&MacLow::fast_ack_timeout, this);
		Simulator::insert_in_s (timerDelay, m_fast_ack_timeout_event);
	} else if (waitSuperFastAck ()) {
		double timerDelay = txDuration + get_pifs ();
		assert (m_super_fast_ack_timeout_event == 0);
		m_super_fast_ack_timeout_event = make_event (&MacLow::super_fast_ack_timeout, this);
		Simulator::insert_in_s (timerDelay, m_super_fast_ack_timeout_event);
	}

	double duration;
	if (m_overrideDurationId > 0.0) {
		duration = m_overrideDurationId;
	} else {
		int ackTxMode = getAckTxModeForData (m_current_hdr.get_addr1 (), m_dataTxMode);
		duration = 0.0;
		if (waitAck ()) {
			duration += get_sifs ();
			duration += calculateTxDuration (ackTxMode, get_ack_size ());
		}
		if (m_nextSize > 0) {
			duration += get_sifs ();
			duration += calculateTxDuration (m_nextTxMode, m_nextSize);
			if (waitAck ()) {
				duration += get_sifs ();
				duration += calculateTxDuration (ackTxMode, get_ack_size ());
			}
		}
	}
	m_current_hdr.set_duration_s (duration);

	m_current_packet.add (m_current_hdr);
	ChunkMac80211Fcs fcs;
	m_current_packet.add (&fcs);

	forwardDown (m_current_packet, m_dataTxMode, m_current_hdr);
	m_current_packet = 0;
	m_current_hdr = 0;
}


void
MacLow::send_cts_after_rts (MacAddress source, double duration, int tx_mode)
{
	/* send a CTS when you receive a RTS 
	 * right after SIFS.
	 */
	int ctsTxMode = getCtsTxModeForRts (source, tx_mode);
	TRACE ("tx CTS to " << source << ", mode=", ctsTxMode);
	ChunkMac80211Hdr cts;
	cts.set_type (MAC_80211_CTL_CTS);
	cts.set_addr1 (source);
	duration -= calculateTxDuration (ctsTxMode, get_cts_size ());
	duration -= get_sifs ();
	cts.set_duration_s (duration);

	Packet *packet = new Packet ();
	packet->add (&cts);
	ChunkMac80211Fcs fcs;
	packet->add (&fcs);

	forwardDown (packet, ctsTxMode, &cts);
}

void
MacLow::send_ack_after_data (MacAddress source, double duration, int tx_mode)
{
	/* send an ACK when you receive 
	 * a packet after SIFS. 
	 */
	int ackTxMode = getAckTxModeForData (source, tx_mode);
	TRACE ("tx ACK to " << source << ", mode=" << ackTxMode);
	ChunkMac80211Hdr ack;
	ack.set_addr1 (source);
	duration -= calculateTxDuration (ackTxMode, get_ack_size ());
	duration -= get_size ();
	ack.set_duration_s (duration);

	Packet *packet = new Packet ();
	packet->add (&cts);
	ChunkMac80211Fcs fcs;
	packet->add (&fcs);

	forwardDown (packet, ackTxMode, &ack);
}

void
MacLow::send_data_after_cts (MacAddress source, double duration, int tx_mode)
{
	/* send the third step in a 
	 * RTS/CTS/DATA/ACK hanshake 
	 */
	// XXX use sendDataPacket ?
	assert (m_current_packet != 0);

	TRACE ("tx " << m_current_hdr.get_type_string () << " to " << m_current_hdr.get_addr2 () <<
	       ", mode=" << m_dataTxMode << ", seq=0x"<< m_current_hdr.get_sequence_control () <<
	       ", tid=", << m_current_hdr.get_qos_tid ());
	double txDuration = calculateTxDuration (m_dataTxMode, m_current_packet.get_size () + m_current_hdr.get_size () + 4);
	if (waitNormalAck ()) {
		double timer_delay = txDuration + get_ack_timeout ();
		assert (m_normal_ack_timeout_event == 0);
		m_normal_ack_timeout_event = make_event (&MacLow::normal_ack_timeout, this);
		Simulator::insert_in_s (timer_delay, m_normal_ack_timeout_event);
	} else if (waitFastAck ()) {
		double timer_delay = txDuration + get_pifs ();
		m_fast_ack_timeout_event = make_event (&MacLow::fast_ack_timeout, this);
		Simulator::insert_in_s (timer_delay, m_fast_ack_timeout_event);
	} else if (waitSuperFastAck ()) {
		double timer_delay = txDuration + get_pifs ();
		m_super_fast_ack_timeout_event = make_event (&MacLow::super_fast_ack_timeout, this);
		Simulator::insert_in_s (timer_delay, m_super_fast_ack_timeout_event);
	}

	duration -= txDuration;
	duration -= get_sifs ();
	m_current_hdr.set_duration_s (duration);

	m_current_packet->add (&m_current_hdr);
	ChunkMac80211Fcs fcs;
	m_current_packet->add (&fcs);

	forwardDown (m_currentTxPacket, m_dataTxMode, &m_current_hdr);
	m_current_packet = 0;
	m_current_hdr = 0;
}


void 
MacLow::waitSIFSAfterEndTx (MacCancelableEvent *macEvent)
{
	m_transmissionListener->startNext ();
}

void
MacLow::normal_ack_timeout (void)
{
	m_transmissionListener->missedACK ();
}
void
MacLow::fast_ack_failed_timeout (void)
{
	m_transmissionListener->missedACK ();
	TRACE ("fast Ack busy but missed");
}
void
MacLow::fast_ack_timeout (void)
{
	if (m_interface->phy ()->getState () == Phy80211::IDLE) {
		TRACE ("fast Ack idle missed");
		m_transmissionListener->missedACK ();
	}
}
void
MacLow::super_fast_ack_timeout ()
{
	if (m_interface->phy ()->getState () == Phy80211::IDLE) {
		TRACE ("super fast Ack failed");
		m_transmissionListener->missedACK ();
	} else {
		TRACE ("super fast Ack ok");
		m_transmissionListener->gotACK (0.0, 0);
	}
}

void
MacLow::cts_timeout (MacCancelableEvent *event)
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
MacLow::notifyNav (double nowTime, ChunkMac80211Hdr const *hdr)
{
	/* XXX
	 * We might need to do something special for the
	 * subtle case of RTS/CTS. I don't know what.
	 *
	 * See section 9.9.2.2.1, 802.11e/D12.1
	 */
	assert (m_lastNavStart < nowTime);
	double oldNavStart = m_lastNavStart;
	double oldNavEnd = oldNavStart + m_lastNavDuration;
	double newNavStart = nowTime;
	double duration = hdr->get_duration_s ();

	if (hdr->is_cf_poll () &&
	    hdr->get_addr2 () == m_interface->getBSSID ()) {
		m_lastNavStart = newNavStart;
		m_lastNavDuration = duration;
		for (NavListenersCI i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
			// XXX !!!!!!!
			(*i)->navReset (newNavStart, duration);
		}
		return;
	}

	if (oldNavEnd > newNavStart) {
		double newNavEnd = newNavStart + duration;
		/* The two NAVs overlap */
		if (newNavEnd > oldNavEnd) {
			double delta = newNavEnd - oldNavEnd;
			m_lastNavDuration += delta;
			for (NavListenersCI i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
				(*i)->navContinue (delta);
			}
		}
	} else {
		m_lastNavStart = newNavStart;
		m_lastNavDuration = duration;
		for (NavListenersCI i = m_navListeners.begin (); i != m_navListeners.end (); i++) {
			(*i)->navStart (newNavStart, duration);
		}
	}
}

double
MacLow::calculateOverallCurrentTxTime (void)
{
	double txTime = 0.0;
	if (m_sendRTS) {
		txTime += calculateTxDuration (m_rtsTxMode, get_rts_size ());
		txTime += calculateTxDuration (m_rtsTxMode, get_cts_size ());
		txTime += get_sifs () * 2;
	}
	txTime += calculateTxDuration (m_dataTxMode, m_current_packet->get_size () + m_current_hdr.get_size () + 4);
	if (waitAck ()) {
		int ackTxMode = getAckTxModeForData (m_current_hdr.get_addr1 (), m_dataTxMode);
		txTime += get_sifs ();
		txTime += calculateTxDuration (ackTxMode, get_ack_size ());
	}
	return txTime;
}

bool
MacLow::waitAck (void)
{
	if (m_waitAck == ACK_NONE) {
		return false;
	} else {
		return true;
	}
}
bool
MacLow::waitNormalAck (void)
{
	if (m_waitAck == ACK_NORMAL) {
		return true;
	} else {
		return false;
	}
}
bool
MacLow::waitFastAck (void)
{
	if (m_waitAck == ACK_FAST) {
		return true;
	} else {
		return false;
	}
}
bool
MacLow::waitSuperFastAck (void)
{
	if (m_waitAck == ACK_SUPER_FAST) {
		return true;
	} else {
		return false;
	}
}

/****************************************************************************
 *        API methods below.
 ****************************************************************************/

void 
MacLow::setData (Packet *packet, ChunkMac80211Hdr const*hdr)
{
	if (m_current_packet != 0) {
		/* currentTxPacket is not NULL because someone started
		 * a transmission and was interrupted before one of:
		 *   - ctsTimeout
		 *   - sendDataAfterCTS
		 * expired. This means that one of these timers is still
		 * running. When the caller of this method also calls
		 * setTransmissionListener, it will trigger a positive
		 * check in maybeCancelPrevious (because of at least one
		 * of these two timer) which will trigger a call to the
		 * previous listener's cancel method. That method is 
		 * responsible for freeing the Packet because it has not 
		 * been forwarded to the Phy yet.
		 *
		 * This typically happens because the high-priority 
		 * QapScheduler has taken access to the channel from
		 * one of the Edca of the QAP.
		 */
		m_current_packet = 0;
	}
	m_current_packet = packet;
	m_current_hdr = *hdr;
}

void 
MacLow::enableNextData (uint32_t size, int txMode)
{
	m_nextSize = size;
	m_nextTxMode = txMode;
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
	assert (m_interface->phy ()->getState () == Phy80211::IDLE);

	TRACE ("startTx size="<< get_current_size () << ", to " << m_current_hdr.get_addr1());

	if (m_nextSize > 0 && !waitAck ()) {
		// we need to start the afterSIFS timeout now.
		double delay = calculateOverallCurrentTxTime ();
		delay += get_sifs ();
		assert (m_wait_sifs_event == 0);
		m_wait_sifs_event = make_cancellable_event (&MacLow::wait_sifs_after_end_tx, this);
		Simulator::insert_in_s (delay, m_wait_sifs_event);
	}

	if (m_sendRTS) {
		send_rts_for_packet ();
	} else {
		send_data_packet ();
	}
	/* When this method completes, we have taken ownership of the medium. */
	assert (m_interface->phy ()->getState () == Phy80211::TX);	
}

void 
MacLow::enableSuperFastAck (void)
{
	m_waitAck = ACK_SUPER_FAST;
}
void 
MacLow::enableFastAck (void)
{
	m_waitAck = ACK_FAST;
}
void 
MacLow::enableACK (void)
{
	m_waitAck = ACK_NORMAL;
}
void 
MacLow::disableACK (void)
{
	m_waitAck = ACK_NONE;
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
	maybeCancelPrevious ();
	m_transmissionListener = listener;
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
MacLow::receive_error (Packet *packet)
{
	TRACE ("rx failed ");
	m_drop_error->log (packet);
	if (waitFastAck ()) {
		assert (m_fast_ack_failed_timeout_event == 0);
		m_fast_ack_failed_timeout_event = make_cancellable_event (&MacLow::fast_ack_failed_timeout, this);
		Simulator::insert_in_s (get_sifs (), m_fast_ack_failed_timeout_event);
	}
	return;
}

void 
MacLow::receive_ok (Packet *packet, double rx_snr, int tx_mode)
{
	/* A packet is received from the PHY.
	 * When we have handled this packet,
	 * we handle any packet present in the
	 * packet queue.
	 */
	ChunkMac80211Hdr hdr;
	packet->remove (&hdr);
	ChunkMac80211Fcs fcs;
	packet->remove (&fcs);
	
	//Tag80211 *tag = static_cast<Tag80211 *> (packet->peek_tag (Tag80211::get_tag ()));
	// XXX
	//m_interface->stations ()->lookup (getSource (packet))->reportRxOk (tag->get_rx_snr (), tag->get_tx_mode ());

	bool isPrevNavZero = isNavZero (now ());
	TRACE ("duration/id=" << hdr.get_duration ());
	notifyNav (now (), &hdr);
	if (hdr.is_rts ()) {
		/* XXX see section 9.9.2.2.1 802.11e/D12.1 */
		if (isPrevNavZero &&
		    hdr.get_addr1 () == getSelf ()) {
			TRACE ("rx RTS from " << hdr.get_addr2 () << ", schedule CTS");
			assert (m_send_cts_event == 0);
			m_send_cts_event = make_event (&MacLow::send_cts_after_rts, this, );
			Simulator::insert_in_s (m_interface->parameters ()->getSIFS (),
						m_send_cts_event);
		} else {
			TRACE ("rx RTS from " << hdr.get_addr2 () << ", cannot schedule CTS");
		}
	} else if (hdr.is_cts () &&
		   hdr.get_addr1 () == getSelf () &&
		   m_cts_timeout_event != 0 &&
		   m_currentTxPacket) {
		TRACE ("rx CTS");
		m_cts_timeout_event->cancel ();
		m_cts_timeout_event = 0;
		m_transmissionListener->gotCTS (rx_snr, tx_mode);
		assert (m_send_data_event == 0);
		m_send_data_event = make_event (&MacLow::send_data_after_cts, this, 
						hdr.get_addr1 (),
						hdr.get_duration_s (),
						tx_mode);
		Simulator::insert_in_s (m_interface->parameters ()->getSIFS (),
					m_send_data_event);
	} else if (hdr.is_ack () &&
		   hdr.get_addr1 () == getSelf () &&
		   waitAck ()) {
		TRACE ("rx ACK");
		if (waitNormalAck ()) {
			m_normal_ack_timeout_event->cancel ();
			m_normal_ack_timeout_event = 0;
		}
		if (waitNormalAck () || waitFastAck ()) {
			m_transmissionListener->gotACK (rx_snr, tx_mode);
		}
		if (m_nextSize > 0) {
			Simulator::insert_in_s (m_interface->parameters ()->getSIFS (),
						make_event (&MacLow::wait_sifs_after_end_tx, this));
		}
	} else if (hdr.is_ctl ()) {
		TRACE ("rx drop " << hdr.get_type_string ());
	} else if (hdr.get_addr1 () == getSelf ()) {
			if (hdr.is_qos_data () && hdr.is_qos_no_ack ()) {
				TRACE ("rx unicast/no_ack from "<<hdr.get_addr2 ());
			} else if (hdr.is_data () || hdr.is_mgt ()) {
				TRACE ("rx unicast/send_ack from " << hdr.get_addr2 ());
				assert (m_send_ack_after_data == 0);
				m_send_ack_event = make_event (&MacLow::send_ack_after_data, this);
				Simulator::insert_in_s (m_interface->parameters ()->getSIFS (),
							m_send_ack_event);
			}
			forward_up (packet, &hdr);
	} else if (hdr.get_addr1 ().is_broadcast ()) {
		if (hdr.is_data () || hdr.is_mgt ()) {
			TRACE ("rx broadcast from " << hdr.get_source ());
			forward_up (packet, &hdr);
		} else {
			// DROP.
		}
	} else {
		//TRACE_VERBOSE ("rx not-for-me from %d", getSource (packet));
	}
	return;
}



