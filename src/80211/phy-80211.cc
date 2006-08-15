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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "phy-80211.h"
#include "bpsk-mode.h"
#include "qam-mode.h"
#include "propagation-model.h"
#include "simulator.h"
#include "packet.h"
#include "random-uniform.h"
#include "count-ptr-holder.tcc"
#include "event.tcc"
#include "trace-container.h"

#include <cassert>
#include <math.h>


#define nopePHY80211_DEBUG 1
#define nopePHY80211_STATE_DEBUG 1

/* All the state transitions are marked by these macros. */
#ifdef PHY80211_STATE_DEBUG
#include <iostream>
#  define STATE_FROM(from) \
std::cout << "PHY self=" << this << " old=" << state_to_string (from);
#  define STATE_TO(to) \
std::cout << " new=" << state_to_string (to);
#  define STATE_AT(at) \
std::cout << " at=" << at << std::endl;
#else
#  define STATE_FROM(from)
#  define STATE_TO(from)
#  define STATE_AT(at)
#endif

#ifdef PHY80211_DEBUG
#include <iostream>
#  define TRACE(x) \
std::cout << "PHY80211 TRACE " << Simulator::now_s () << " " << x << std::endl;
#else
#  define TRACE(x)
#endif

#ifndef max
#define max(a,b) (((a)>(b))?a:b)
#endif /* max */



namespace yans {

/****************************************************************
 *       This destructor is needed.
 ****************************************************************/

Phy80211Listener::~Phy80211Listener ()
{}


/****************************************************************
 *       Phy event class
 ****************************************************************/

class RxEvent {
public:
	RxEvent (uint32_t size, uint8_t payload_mode, 
		 uint64_t duration_us, double rx_power)
		: m_size (size),
		  m_payload_mode (payload_mode),
		  m_start_time_us (Simulator::now_us ()),
		  m_end_time_us (m_start_time_us + duration_us),
		  m_rx_power_w (rx_power),
		  m_ref_count (1)
	{}
	~RxEvent ()
	{}
	
	void ref (void) {
		m_ref_count++;
	}
	void unref (void) {
		m_ref_count--;
		if (m_ref_count == 0) {
			delete this;
		}
	}
	uint64_t get_duration_us (void) const {
		return m_end_time_us - m_start_time_us;
	}
	uint64_t get_start_time_us (void) const {
		return m_start_time_us;
	}
	uint64_t get_end_time_us (void) const {
		return m_end_time_us;
	}
	bool overlaps (uint64_t time_us) const {
		if (m_start_time_us <= time_us &&
		    m_end_time_us >= time_us) {
			return true;
		} else {
			return false;
		}
	}
	double get_rx_power_w (void) const {
		return m_rx_power_w;
	}
	uint32_t get_size (void) const {
		return m_size;
	}
	uint8_t get_payload_mode (void) const {
		return m_payload_mode;
	}
	uint8_t get_header_mode (void) const {
		return 0;
	}

private:
	uint32_t m_size;
	uint8_t m_payload_mode;
	uint64_t m_start_time_us;
	uint64_t m_end_time_us;
	double m_rx_power_w;
	int m_ref_count;
};


/****************************************************************
 *       Class which records SNIR change events for a 
 *       short period of time.
 ****************************************************************/

Phy80211::NiChange::NiChange (uint64_t time, double delta)
	: m_time (time), m_delta (delta) 
{}
uint64_t
Phy80211::NiChange::get_time_us (void) const
{
	return m_time;
}
double 
Phy80211::NiChange::get_delta (void) const
{
	return m_delta;
}
bool 
Phy80211::NiChange::operator < (Phy80211::NiChange const &o) const
{
	return (m_time < o.m_time)?true:false;
}



/****************************************************************
 *       The actual Phy80211 class
 ****************************************************************/

Phy80211::Phy80211 ()
	: m_syncing (false),
	  m_end_tx_us (0),
	  m_end_sync_us (0),
	  m_end_cca_busy_us (0),
	  m_start_tx_us (0),
	  m_start_sync_us (0),
	  m_start_cca_busy_us (0),
	  m_previous_state_change_time_us (0),
	  m_end_sync_event (),
	  m_random (new RandomUniform ())
{}

Phy80211::~Phy80211 ()
{
	delete m_random;
	EventsI i = m_events.begin ();
	while (i != m_events.end ()) {
		(*i)->unref ();
		i++;
	}
	m_events.erase (m_events.begin (), m_events.end ());
	for (ModesCI j = m_modes.begin (); j != m_modes.end (); j++) {
		delete (*j);
	}
	m_modes.erase (m_modes.begin (), m_modes.end ());
}

void 
Phy80211::register_traces (TraceContainer *container)
{
	container->register_callback ("80211-rx-start", &m_start_rx_logger);
	container->register_callback ("80211-sync-start", &m_start_sync_logger);
	container->register_callback ("80211-sync-end", &m_end_sync_logger);
	container->register_callback ("80211-tx-start", &m_start_tx_logger);
	container->register_callback ("80211-phy-state", &m_state_logger);
}

void 
Phy80211::set_propagation_model (PropagationModel *propagation)
{
	m_propagation = propagation;
}

void 
Phy80211::set_receive_ok_callback (SyncOkCallback callback)
{
	m_sync_ok_callback = callback;
}
void 
Phy80211::set_receive_error_callback (SyncErrorCallback callback)
{
	m_sync_error_callback = callback;
}
void 
Phy80211::receive_packet (Packet const packet, 
			  double rx_power_w,
			  uint8_t tx_mode,
			  uint8_t stuff)
{
	uint64_t rx_duration_us = calculate_tx_duration_us (packet.get_size (), tx_mode);
	uint64_t end_rx = now_us () + rx_duration_us;
	m_start_rx_logger (rx_duration_us, rx_power_w);

	RxEvent *event = new RxEvent (packet.get_size (), 
				      tx_mode,
				      rx_duration_us,
				      rx_power_w);
	append_event (event);

	switch (get_state ()) {
	case Phy80211::SYNC:
		TRACE ("drop packet because already in sync (power="<<
		       rx_power_w<<"W)");
		if (end_rx > m_end_sync_us) {
			goto maybe_cca_busy;
		}
		break;
	case Phy80211::TX:
		TRACE ("drop packet because already in tx (power="<<
		       rx_power_w<<"W)");
		if (end_rx > m_end_tx_us) {
			goto maybe_cca_busy;
		}
		break;
	case Phy80211::CCA_BUSY:
	case Phy80211::IDLE:
		if (rx_power_w > m_ed_threshold_w) {
			// sync to signal
			notify_sync_start (rx_duration_us);
			switch_to_sync (rx_duration_us);
			m_start_sync_logger (rx_duration_us, rx_power_w);
			assert (!m_end_sync_event.is_running ());
			m_end_sync_event = make_event (&Phy80211::end_sync, this, 
						     packet,
						     make_count_ptr_holder (event), 
						     stuff);
			Simulator::schedule_rel_us (rx_duration_us, m_end_sync_event);
		} else {
			TRACE ("drop packet because signal power too small ("<<
			       rx_power_w<<"<"<<m_ed_threshold_w<<")");
			goto maybe_cca_busy;
		}
	break;
	}

	event->unref ();
	return;

 maybe_cca_busy:

	if (rx_power_w > m_ed_threshold_w) {
		switch_maybe_to_cca_busy (rx_duration_us);
		notify_cca_busy_start (rx_duration_us);
	} else {
		double threshold = m_ed_threshold_w - rx_power_w;
		NiChanges ni;
		calculate_noise_interference_w (event, &ni);
		double noise_interference_w = 0.0;
		uint64_t end = now_us ();
		for (NiChangesI i = ni.begin (); i != ni.end (); i++) {
			noise_interference_w += i->get_delta ();
			if (noise_interference_w < threshold) {
				break;
			}
			end = i->get_time_us ();
		}
		if (end > now_us ()) {
			switch_maybe_to_cca_busy (end - now_us ());
			notify_cca_busy_start (end - now_us ());
		}
	}

	event->unref ();
}
void 
Phy80211::send_packet (Packet const packet, uint8_t tx_mode, uint8_t tx_power, uint8_t stuff)
{
	/* Transmission can happen if:
	 *  - we are syncing on a packet. It is the responsability of the
	 *    MAC layer to avoid doing this but the PHY does nothing to 
	 *    prevent it.
	 *  - we are idle
	 */
	assert (!is_state_tx ());

	if (is_state_sync ()) {
		m_end_sync_event.cancel ();
	}

	uint64_t tx_duration_us = calculate_tx_duration_us (packet.get_size (), tx_mode);
	m_start_tx_logger (tx_duration_us, get_mode_bit_rate (tx_mode), get_power_dbm (tx_power));
	notify_tx_start (tx_duration_us);
	switch_to_tx (tx_duration_us);
	m_propagation->send (packet, get_power_dbm (tx_power), tx_mode, stuff);
}

void 
Phy80211::set_ed_threshold_dbm (double ed_threshold)
{
	m_ed_threshold_w = dbm_to_w (ed_threshold);
}
void 
Phy80211::set_rx_noise_db (double rx_noise)
{
	m_rx_noise_ratio = db_to_ratio (rx_noise);
}
void 
Phy80211::set_tx_power_increments_dbm (double tx_power_base, 
				       double tx_power_end, 
				       int n_tx_power)
{
	m_tx_power_base_dbm = tx_power_base;
	m_tx_power_end_dbm = tx_power_end;
	m_n_tx_power = n_tx_power;
}
uint32_t 
Phy80211::get_n_modes (void) const
{
	return m_modes.size ();
}
uint32_t 
Phy80211::get_mode_bit_rate (uint8_t mode) const
{
	return get_mode (mode)->get_rate ();
}
uint32_t 
Phy80211::get_n_txpower (void) const
{
	return m_n_tx_power;
}

double 
Phy80211::calculate_snr (uint8_t tx_mode, double ber) const
{
	return get_snr_for_ber (get_mode (tx_mode), ber);;
}

double 
Phy80211::get_snr_for_ber (TransmissionMode *mode, double ber) const
{
	double low, high, precision;
	low = 1e-25;
	high = 1e25;
	precision = 1e-12;
	while (high - low > precision) {
		assert (high >= low);
		double middle = low + (high - low) / 2;
		if ((1 - mode->get_chunk_success_rate (middle, 1)) > ber) {
			low = middle;
		} else {
			high = middle;
		}
	}
	return low;
}

void
Phy80211::configure_80211a (void)
{
	m_plcp_preamble_delay_us = 20;
	m_plcp_header_length = 4 + 1 + 12 + 1 + 6 + 16 + 6;
	/* 4095 bytes at a 6Mb/s rate with a 1/2 coding rate. */
	m_max_packet_duration_us = (uint64_t)(1000000 * 4095.0*8.0/6000000.0*(1.0/2.0));

	add_tx_rx_mode (new FecBpskMode (20e6, 6000000, 0.5,   10, 11));
	add_tx_rx_mode (new FecBpskMode (20e6, 9000000, 0.75,  5, 8));
	add_tx_rx_mode (new FecQamMode (20e6, 12000000, 0.5,   4, 10, 11, 0));
	add_tx_rx_mode (new FecQamMode (20e6, 18000000, 0.75,  4, 5, 8, 31));
	//add_tx_rx_mode (new FecQamMode (20e6, 24000000, 0.5,   16, 10, 11, 0));
	add_tx_rx_mode (new FecQamMode (20e6, 36000000, 0.75,  16, 5, 8, 31));
	//add_tx_rx_mode (new FecQamMode (20e6, 48000000, 0.666, 64, 6, 1, 16));
	add_tx_rx_mode (new FecQamMode (20e6, 54000000, 0.75,  64, 5, 8, 31));

#ifdef PHY80211_DEBUG
	for (double db = 0; db < 30; db+= 0.5) {
		std::cout <<db<<" ";
		for (uint8_t i = 0; i < get_n_modes (); i++) {
			TransmissionMode *mode = get_mode (i);
			double ber = 1-mode->get_chunk_success_rate (db_to_ratio (db), 1);
			std::cout <<ber<< " ";
		}
		std::cout << std::endl;
	}
#endif
}

void 
Phy80211::register_listener (Phy80211Listener *listener)
{
	m_listeners.push_back (listener);
}

bool 
Phy80211::is_state_cca_busy (void)
{
	return get_state () == CCA_BUSY;
}

bool 
Phy80211::is_state_idle (void)
{
	return (get_state () == IDLE)?true:false;
}
bool 
Phy80211::is_state_busy (void)
{
	return (get_state () != IDLE)?true:false;
}
bool 
Phy80211::is_state_sync (void)
{
	return (get_state () == SYNC)?true:false;
}
bool 
Phy80211::is_state_tx (void)
{
	return (get_state () == TX)?true:false;
}

uint64_t
Phy80211::get_state_duration_us (void)
{
	return now_us () - m_previous_state_change_time_us;
}
uint64_t
Phy80211::get_delay_until_idle_us (void)
{
	int64_t retval_us;

	switch (get_state ()) {
	case SYNC:
		retval_us = m_end_sync_us - now_us ();
		break;
	case TX:
		retval_us = m_end_tx_us - now_us ();
		break;
	case CCA_BUSY:
		retval_us = m_end_cca_busy_us - now_us ();
		break;
	case IDLE:
		retval_us = 0;
		break;
	default:
		assert (false);
		// NOTREACHED
		retval_us = 0;
		break;
	}
	retval_us = max (retval_us, 0);
	return (uint64_t)retval_us;
}


uint64_t 
Phy80211::calculate_tx_duration_us (uint32_t size, uint8_t payload_mode) const
{
	uint64_t delay = m_plcp_preamble_delay_us;
	delay += m_plcp_header_length / get_mode (0)->get_data_rate ();
	uint64_t tmp = size * 8;
	tmp *= 1000000;
	delay +=  tmp / get_mode (payload_mode)->get_data_rate ();
	return delay;
}

char const *
Phy80211::state_to_string (enum Phy80211State state)
{
	switch (state) {
	case TX:
		return "TX";
		break;
	case CCA_BUSY:
		return "CCA_BUSY";
		break;
	case IDLE:
		return "IDLE";
		break;
	case SYNC:
		return "SYNC";
		break;
	default:
		return "XXX";
		break;
	}
}
enum Phy80211::Phy80211State 
Phy80211::get_state (void)
{
	if (m_end_tx_us > now_us ()) {
		return Phy80211::TX;
	} else if (m_syncing) {
		return Phy80211::SYNC;
	} else if (m_end_cca_busy_us > now_us ()) {
		return Phy80211::CCA_BUSY;
	} else {
		return Phy80211::IDLE;
	}
}

double 
Phy80211::db_to_ratio (double dB) const
{
	double ratio = pow(10.0,dB/10.0);
	return ratio;
}

double 
Phy80211::dbm_to_w (double dBm) const
{
	double mW = pow(10.0,dBm/10.0);
	return mW / 1000.0;
}

double
Phy80211::get_ed_threshold_w (void) const
{
	return m_ed_threshold_w;
}

uint64_t
Phy80211::now_us (void) const
{
	return Simulator::now_us ();
}
uint64_t 
Phy80211::get_max_packet_duration_us (void) const
{
	return m_max_packet_duration_us;
}

void
Phy80211::add_tx_rx_mode (TransmissionMode *mode)
{
	m_modes.push_back (mode);
}

TransmissionMode *
Phy80211::get_mode (uint8_t mode) const
{
	return m_modes[mode];
}

double 
Phy80211::get_power_dbm (uint8_t power) const
{
	assert (m_tx_power_base_dbm <= m_tx_power_end_dbm);
	assert (m_n_tx_power > 0);
	double dbm = m_tx_power_base_dbm + (m_tx_power_end_dbm - m_tx_power_base_dbm) / m_n_tx_power;
	return dbm;
}

void 
Phy80211::notify_tx_start (uint64_t duration_us)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_tx_start (duration_us);
	}
}
void 
Phy80211::notify_sync_start (uint64_t duration_us)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_rx_start (duration_us);
	}
}
void 
Phy80211::notify_sync_end_ok (void)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_rx_end_ok ();
	}
}
void 
Phy80211::notify_sync_end_error (void)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_rx_end_error ();
	}
}
void 
Phy80211::notify_cca_busy_start (uint64_t duration_us)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_cca_busy_start (duration_us);
	}
}

void
Phy80211::log_previous_idle_and_cca_busy_states (void)
{
	uint64_t now = now_us ();
	uint64_t idle_start = max (m_end_cca_busy_us, m_end_sync_us);
	idle_start = max (idle_start, m_end_tx_us);
	assert (idle_start <= now);
	if (m_end_cca_busy_us > m_end_sync_us && 
	    m_end_cca_busy_us > m_end_tx_us) {
		uint64_t cca_busy_start = max (m_end_tx_us, m_end_sync_us);
		cca_busy_start = max (cca_busy_start, m_start_cca_busy_us);
			m_state_logger (cca_busy_start, idle_start - cca_busy_start, 2);
	}
	m_state_logger (idle_start, now - idle_start, 3);
}

void
Phy80211::switch_to_tx (uint64_t tx_duration_us)
{
	uint64_t now = now_us ();
	switch (get_state ()) {
	case Phy80211::SYNC:
		/* The packet which is being received as well
		 * as its end_sync event are cancelled by the caller.
		 */
		m_syncing = false;
		m_state_logger (m_start_sync_us, now - m_start_sync_us, 1);
		break;
	case Phy80211::CCA_BUSY: {
		uint64_t cca_start = max (m_end_sync_us, m_end_tx_us);
		cca_start = max (cca_start, m_start_cca_busy_us);
		m_state_logger (cca_start, now - cca_start, 2);
	} break;
	case Phy80211::IDLE:
		log_previous_idle_and_cca_busy_states ();
		break;
	default:
		assert (false);
		break;
	}
	m_state_logger (now, tx_duration_us, 0);
	m_previous_state_change_time_us = now;
	m_end_tx_us = now + tx_duration_us;
	m_start_tx_us = now;
}
void
Phy80211::switch_to_sync (uint64_t rx_duration_us)
{
	assert (is_state_idle () || is_state_cca_busy ());
	assert (!m_syncing);
	uint64_t now = now_us ();
	switch (get_state ()) {
	case Phy80211::IDLE:
		log_previous_idle_and_cca_busy_states ();
		break;
	case Phy80211::CCA_BUSY: {
		uint64_t cca_start = max (m_end_sync_us, m_end_tx_us);
		cca_start = max (cca_start, m_start_cca_busy_us);
		m_state_logger (cca_start, now - cca_start, 2);
	} break;
	case Phy80211::SYNC:
	case Phy80211::TX:
		assert (false);
		break;
	}
	m_previous_state_change_time_us = now;
	m_syncing = true;
	m_start_sync_us = now;
	m_end_sync_us = now + rx_duration_us;
	assert (is_state_sync ());
}
void
Phy80211::switch_from_sync (void)
{
	assert (is_state_sync ());
	assert (m_syncing);

	uint64_t now = now_us ();
	m_state_logger (m_start_sync_us, now - m_start_sync_us, 1);
	m_previous_state_change_time_us = now;
	m_syncing = false;

	assert (is_state_idle () || is_state_cca_busy ());
}
void
Phy80211::switch_maybe_to_cca_busy (uint64_t duration_us)
{
	uint64_t now = now_us ();
	switch (get_state ()) {
	case Phy80211::IDLE:
		log_previous_idle_and_cca_busy_states ();
	break;
	case Phy80211::CCA_BUSY:
		break;
	case Phy80211::SYNC:
		break;
	case Phy80211::TX:
		break;
	}
	m_start_cca_busy_us = now;
	m_end_cca_busy_us = max (m_end_cca_busy_us, now + duration_us);
}

void 
Phy80211::append_event (RxEvent *event)
{
	/* attempt to remove the events which are 
	 * not useful anymore. 
	 * i.e.: all events which end _before_
	 *       now - m_maxPacketDuration
	 */
	
	if (now_us () > get_max_packet_duration_us ()) {
		double end_us = now_us () - get_max_packet_duration_us ();
		EventsI i = m_events.begin ();
		while (i != m_events.end () &&
		       (*i)->get_end_time_us () <= end_us) {
			(*i)->unref ();
			i++;
		}
		m_events.erase (m_events.begin (), i);
	} 
	event->ref ();
	m_events.push_back (event);
}



/**
 * Stuff specific to the BER model here.
 */

double
Phy80211::calculate_snr (double signal, double noise_interference, TransmissionMode *mode) const
{
	// thermal noise at 290K in J/s = W
	static const double BOLTZMANN = 1.3803e-23;
	double Nt = BOLTZMANN * 290.0 * mode->get_signal_spread ();
	// receiver noise floor (W)
	double noise_floor = m_rx_noise_ratio * Nt;
	double noise = noise_floor + noise_interference;
	double snr = signal / noise;
	return snr;
}

double
Phy80211::calculate_noise_interference_w (RxEvent *event, NiChanges *ni) const
{
	EventsCI i = m_events.begin ();
	double noise_interference = 0.0;
	while (i != m_events.end ()) {
		if (event == (*i)) {
			i++;
			continue;
		}
		if (event->overlaps ((*i)->get_start_time_us ())) {
			ni->push_back (NiChange ((*i)->get_start_time_us (), (*i)->get_rx_power_w ()));
		}
		if (event->overlaps ((*i)->get_end_time_us ())) {
			ni->push_back (NiChange ((*i)->get_end_time_us (), -(*i)->get_rx_power_w ()));
		}
		if ((*i)->overlaps (event->get_start_time_us ())) {
			noise_interference += (*i)->get_rx_power_w ();
		}
		i++;
	}
	ni->push_back (NiChange (event->get_start_time_us (), noise_interference));
	ni->push_back (NiChange (event->get_end_time_us (), 0));

	/* quicksort vector of NI changes by time. */
	std::sort (ni->begin (), ni->end (), std::less<NiChange> ());

	return noise_interference;
}

double
Phy80211::calculate_chunk_success_rate (double snir, uint64_t delay, TransmissionMode *mode) const
{
	if (delay == 0) {
		return 1.0;
	}
	uint32_t rate = mode->get_rate ();
	uint64_t nbits = rate * delay / 1000000;
	double csr = mode->get_chunk_success_rate (snir, (uint32_t)nbits);
	return csr;
}

double 
Phy80211::calculate_per (RxEvent const *event, NiChanges *ni) const
{	
	double psr = 1.0; /* Packet Success Rate */
	NiChangesI j = ni->begin ();
	uint64_t previous_us = (*j).get_time_us ();
	uint64_t plcp_header_start_us = (*j).get_time_us () + m_plcp_preamble_delay_us;
	uint64_t plcp_payload_start_us = plcp_header_start_us + 
		m_plcp_header_length * get_mode (event->get_header_mode ())->get_data_rate () / 1000000;
	double noise_interference_w = (*j).get_delta ();
	double power_w = event->get_rx_power_w ();
	TransmissionMode *payload_mode = get_mode (event->get_payload_mode ());
	TransmissionMode *header_mode = get_mode (event->get_header_mode ());

	j++;
	while (ni->end () != j) {
		uint64_t current_us = (*j).get_time_us ();
		assert (current_us >= previous_us);
		
		if (previous_us >= plcp_payload_start_us) {
			psr *= calculate_chunk_success_rate (calculate_snr (power_w, 
									    noise_interference_w, payload_mode), 
							     current_us - previous_us,
							     payload_mode);
		} else if (previous_us >= plcp_header_start_us) {
			if (current_us >= plcp_payload_start_us) {
				psr *= calculate_chunk_success_rate (calculate_snr (power_w, 
										    noise_interference_w, 
										    header_mode), 
								     plcp_payload_start_us - previous_us,
								     header_mode);
				psr *= calculate_chunk_success_rate (calculate_snr (power_w, 
										    noise_interference_w, 
										    payload_mode),
								     current_us - plcp_payload_start_us,
								     payload_mode);
			} else {
				assert (current_us >= plcp_header_start_us);
				psr *= calculate_chunk_success_rate (calculate_snr (power_w, 
										    noise_interference_w, 
										    header_mode), 
								     current_us - previous_us,
								     header_mode);
			}
		} else {
			if (current_us >= plcp_payload_start_us) {
				psr *= calculate_chunk_success_rate (calculate_snr (power_w, 
										    noise_interference_w, 
										    header_mode), 
								     plcp_payload_start_us - plcp_header_start_us,
								     header_mode);
				psr *= calculate_chunk_success_rate (calculate_snr (power_w, 
										    noise_interference_w, 
										    payload_mode), 
								     current_us - plcp_payload_start_us,
								     payload_mode);
			} else if (current_us >= plcp_header_start_us) {
				psr *= calculate_chunk_success_rate (calculate_snr (power_w, 
										    noise_interference_w, 
										    header_mode), 
								     current_us - plcp_header_start_us,
								     header_mode);
			}
		}

		noise_interference_w += (*j).get_delta ();
		previous_us = (*j).get_time_us ();
		j++;
	}

	double per = 1 - psr;
	return per;
}


void
Phy80211::end_sync (Packet const packet, CountPtrHolder<RxEvent> ev, uint8_t stuff)
{
	RxEvent *event = ev.remove ();
	assert (is_state_sync ());
	assert (event->get_end_time_us () == now_us ());

	NiChanges ni;
	double noise_interference_w = calculate_noise_interference_w (event, &ni);
	double snr = calculate_snr (event->get_rx_power_w (),
				    noise_interference_w,
				    get_mode (event->get_payload_mode ()));
	
	/* calculate the SNIR at the start of the packet and accumulate
	 * all SNIR changes in the snir vector.
	 */
	double per = calculate_per (event, &ni);
	TRACE ("mode="<<((uint32_t)event->get_payload_mode ())<<
	       ", ber="<<(1-get_mode (event->get_payload_mode ())->get_chunk_success_rate (snr, 1))<<
	       ", snr="<<snr<<", per="<<per<<", size="<<packet.get_size ());
	
	if (m_random->get_double () > per) {
		m_end_sync_logger (true);
		notify_sync_end_ok ();
		switch_from_sync ();
		m_sync_ok_callback (packet, snr, event->get_payload_mode (), stuff);
	} else {
		/* failure. */
		m_end_sync_logger (false);
		notify_sync_end_error ();
		switch_from_sync ();
		m_sync_error_callback (packet, snr);
	}
	event->unref ();
}




}; // namespace yans
