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
#include "cancellable-event.tcc"
#include "random-uniform.h"

#include <cassert>
#include <math.h>


#define PHY80211_DEBUG 1
#define nopePHY80211_STATE_DEBUG 1

/* All the state transitions are marked by these macros. */
#ifdef PHY80211_STATE_DEBUG
#include <iostream>
#  define STATE_FROM(from) \
std::cout << "PHY TRACE " << get_self_address () << " from " << state_to_string (from);
#  define STATE_TO(to) \
std::cout << " to " << stateToString (to);
#  define STATE_AT(at) \
std::cout << " at " << at << std::endl;
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

class Phy80211::RxEvent {
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
	: m_sleeping (false),
	  m_rxing (false),
	  m_end_tx (0.0),
	  m_previous_state_change_time (0.0),
	  m_rx_ok_callback (0),
	  m_rx_error_callback (0),
	  m_end_rx_event (0),
	  m_random (new RandomUniform ())
{}

Phy80211::~Phy80211 ()
{
	delete m_rx_ok_callback;
	delete m_rx_error_callback;
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
Phy80211::set_propagation_model (PropagationModel *propagation)
{
	m_propagation = propagation;
}

void 
Phy80211::set_receive_ok_callback (RxOkCallback *callback)
{
	m_rx_ok_callback = callback;
}
void 
Phy80211::set_receive_error_callback (RxErrorCallback *callback)
{
	m_rx_error_callback = callback;
}
void 
Phy80211::receive_packet (Packet *packet, 
			  double rx_power_w,
			  uint8_t tx_mode)
{
	uint64_t rx_duration_us = calculate_tx_duration_us (packet->get_size (), tx_mode);

	RxEvent *event = new RxEvent (packet->get_size (), 
				      tx_mode,
				      rx_duration_us,
				      rx_power_w);
	append_event (event);

	switch (get_state ()) {
	case Phy80211::SYNC:
		TRACE ("drop packet because already in sync (power="<<
		       rx_power_w<<"W)");
		break;
	case Phy80211::TX:
		TRACE ("drop packet because already in tx (power="<<
		       rx_power_w<<"W)");
		break;
	case Phy80211::SLEEP:
		TRACE ("drop packet because sleeping");
		break;
	case Phy80211::IDLE: {
		if (rx_power_w > m_ed_threshold_w) {
			// sync to signal
			double rx_duration_s = calculate_tx_duration_s (packet->get_size (), tx_mode);
			notify_rx_start (now_s (), rx_duration_s);
			switch_to_sync_from_idle (rx_duration_s);
			assert (m_end_rx_event == 0);
			event->ref ();
			m_end_rx_event = make_cancellable_event (&Phy80211::end_rx, this, packet, event);
			Simulator::insert_in_us (rx_duration_us, m_end_rx_event);
		} else {
			TRACE ("drop packet because signal power too small ("<<
			       rx_power_w<<"<"<<m_ed_threshold_w<<")");
		}
	} break;
	}

	event->unref ();
}
void 
Phy80211::send_packet (Packet *packet, uint8_t tx_mode, uint8_t tx_power)
{
	/* Transmission can happen if:
	 *  - we are syncing on a packet. It is the responsability of the
	 *    MAC layer to avoid doing this but the PHY does nothing to 
	 *    prevent it.
	 *  - we are idle
	 */
	assert (is_state_idle () || is_state_rx ());

	if (is_state_rx ()) {
		m_end_rx_event->cancel ();
	}

	double tx_duration = calculate_tx_duration_s (packet->get_size (), tx_mode);
	notify_tx_start (now_s (), tx_duration);
	switch_to_tx (tx_duration);
	m_propagation->send (packet, get_power_dbm (tx_power), tx_mode);
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
Phy80211::get_n_modes (void)
{
	return m_modes.size ();
}
uint32_t 
Phy80211::get_n_txpower (void)
{
	return m_n_tx_power;
}

void
Phy80211::configure_80211a (void)
{
	m_plcp_header_length = 4 + 1 + 12 + 1 + 6 + 16 + 6;
	m_plcp_preamble_delay_us = 20;
	/* 4095 bytes at a 6Mb/s rate with a 1/2 coding rate. */
	m_max_packet_duration_s = 4095.0*8.0/6000000.0*(1.0/2.0);
	add_tx_rx_mode (new FecBpskMode (20e6, 6000000, 0.5,   10, 11));
	add_tx_rx_mode (new FecBpskMode (20e6, 9000000, 0.75,  5, 8));
	add_tx_rx_mode (new FecQamMode (20e6, 12000000, 0.5,   4, 10, 11, 0));
	add_tx_rx_mode (new FecQamMode (20e6, 18000000, 0.75,  4, 5, 8, 31));
	add_tx_rx_mode (new FecQamMode (20e6, 24000000, 0.5,   16, 10, 11, 0));
	add_tx_rx_mode (new FecQamMode (20e6, 36000000, 0.75,  16, 5, 8, 31));
	add_tx_rx_mode (new FecQamMode (20e6, 48000000, 0.666, 64, 6, 1, 16));
	add_tx_rx_mode (new FecQamMode (20e6, 54000000, 0.75,  64, 5, 8, 31));
}

void 
Phy80211::register_listener (Phy80211Listener *listener)
{
	m_listeners.push_back (listener);
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
Phy80211::is_state_rx (void)
{
	return (get_state () == SYNC)?true:false;
}
bool 
Phy80211::is_state_tx (void)
{
	return (get_state () == TX)?true:false;
}
bool 
Phy80211::is_state_sleep (void)
{
	return (get_state () == SLEEP)?true:false;
}

double
Phy80211::get_state_duration (void)
{
	return now_s () - m_previous_state_change_time;
}
double 
Phy80211::get_delay_until_idle (void)
{
	double retval;

	switch (get_state ()) {
	case SYNC:
		retval = m_end_rx - now_s ();
		break;
	case TX:
		retval = m_end_tx - now_s ();
		break;
	case IDLE:
		retval = 0.0;
		break;
	case SLEEP:
		assert (false);
		// quiet compiler.
		retval = 0.0;
		break;
	default:
		assert (false);
		// NOTREACHED
		retval = 0.0;
		break;
	}
	retval = max (retval, 0.0);
	return retval;
}


uint64_t 
Phy80211::calculate_tx_duration_us (uint32_t size, uint8_t payload_mode)
{
	uint64_t delay = m_plcp_preamble_delay_us;
	delay += m_plcp_header_length / get_mode (0)->get_data_rate ();
	delay += 1000000 * (size * 8) / get_mode (payload_mode)->get_data_rate ();
	return delay;
}

double 
Phy80211::calculate_tx_duration_s (uint32_t size, uint8_t payload_mode)
{
	double delay = 	m_plcp_preamble_delay_us * 1e-6;
	delay += m_plcp_header_length / get_mode (0)->get_data_rate ();
	delay += (size * 8) / get_mode (payload_mode)->get_data_rate ();
	return delay;
}



enum Phy80211::Phy80211State 
Phy80211::get_state (void)
{
	if (m_sleeping) {
		assert (m_end_tx == 0);
		assert (!m_rxing);
		return Phy80211::SLEEP;
	} else {
		if (m_end_tx != 0 && m_end_tx > now_s ()) {
			return Phy80211::TX;
		} else if (m_end_tx != 0) {
			/* At one point in the past, we completed
			 * transmission of this packet.
			 */
			STATE_FROM (Phy80211::TX);
			STATE_TO (Phy80211::IDLE);
			STATE_AT (m_end_tx);
			
			m_previous_state_change_time = m_end_tx;
			m_end_tx = 0;
		}
		if (m_rxing) {
			return Phy80211::SYNC;
		} else {
			return Phy80211::IDLE;
		}
	}
}

double 
Phy80211::db_to_ratio (double dB)
{
	double ratio = pow(10.0,dB/10.0);
	return ratio;
}

double 
Phy80211::dbm_to_w (double dBm)
{
	double mW = pow(10.0,dBm/10.0);
	return mW / 1000.0;
}

double
Phy80211::get_ed_threshold_w (void)
{
	return m_ed_threshold_w;
}

double
Phy80211::now_s (void) const
{
	return Simulator::now_s ();
}
uint64_t
Phy80211::now_us (void) const
{
	return Simulator::now_us ();
}
uint64_t 
Phy80211::get_max_packet_duration_us (void) const
{
	return (uint64_t)(m_max_packet_duration_s * 1000000);
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
Phy80211::notify_tx_start (double now, double duration)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_tx_start (now, duration);
	}
}
void 
Phy80211::notify_rx_start (double now, double duration)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_rx_start (now, duration);
	}
}
void 
Phy80211::notify_rx_end (double now, bool receivedOk)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_rx_end (now, receivedOk);
	}
}
void 
Phy80211::notify_sleep (double now)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_sleep (now);
	}
}
void 
Phy80211::notify_wakeup (double now)
{
	for (ListenersCI i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notify_wakeup (now);
	}
}

void
Phy80211::switch_to_tx (double txDuration)
{
	assert (m_end_tx == 0);
	switch (get_state ()) {
	case Phy80211::SYNC:
		/* If we were receiving a packet when this tx
		 * started, we drop it now. It will be discarded 
		 * later in endRx.
		 */
		assert (!m_rxing);
		m_rxing = false;
		STATE_FROM (Phy80211::SYNC);
		break;
	case Phy80211::IDLE:
		STATE_FROM (Phy80211::IDLE);
		break;
	default:
		assert (false);
		break;
	}
	m_previous_state_change_time = now_s ();
	m_end_tx = now_s () + txDuration;
	STATE_TO (Phy80211::TX);
	STATE_AT (now_s ());
}
void
Phy80211::switch_to_sync_from_idle (double rx_duration)
{
	assert (is_state_idle ());
	assert (!m_rxing);
	m_previous_state_change_time = now_s ();
	m_rxing = true;
	m_end_rx = now_s () + rx_duration;
	assert (get_state () == Phy80211::SYNC);
	STATE_FROM (Phy80211::IDLE);
	STATE_TO (Phy80211::SYNC);
	STATE_AT (now_s ());
}
void
Phy80211::switch_to_sleep (void)
{
	assert (!m_sleeping);
	switch (get_state ()) {
	case Phy80211::SYNC:
		/* If we were receiving a packet when this sleep is
		 * started, we drop it now. It will be discarded 
		 * later in endRx.
		 */
		assert (m_rxing);
		m_rxing = false;
		STATE_FROM (Phy80211::SYNC);
		break;
	case Phy80211::IDLE:
		/* */
		STATE_FROM (Phy80211::IDLE);
		break;
	case Phy80211::TX:
		/* If we were transmitting a packet when this sleep
		 * started, we cannot drop it as we should (obviously,
		 * the transmission will not be able to complete)
		 * because the packet has already been put in the 
		 * reception queue of all the target nodes. To be
		 * able to drop it, we would need to remove it from
		 * each target queue or notify each target to remove 
		 * it.
		 * I know, this sucks and it is a bug but there is no
		 * reasonable fix to it.
		 */
		assert (false);
		break;
	default:
		assert (false);
	}
	m_previous_state_change_time = now_s ();
	m_sleeping = true;
	STATE_FROM (Phy80211::SLEEP);
	STATE_AT (now_s ());
}
void
Phy80211::switch_to_idle_from_sleep (void)
{
	assert (is_state_sleep ());
	assert (!m_sleeping);

	m_previous_state_change_time = now_s ();
	m_sleeping = false;

	assert (is_state_idle ());

	STATE_FROM (Phy80211::SLEEP);
	STATE_TO (Phy80211::IDLE);
	STATE_AT (now_s ());
}
void
Phy80211::switch_to_idle_from_sync (void)
{
	assert (is_state_rx ());
	assert (m_rxing);

	m_previous_state_change_time = now_s ();
	m_rxing = false;

	assert (is_state_idle ());

	STATE_FROM (Phy80211::SYNC);
	STATE_TO (Phy80211::IDLE);
	STATE_AT (now_s ());
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
			delete (*i);
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
	uint32_t rate = mode->get_data_rate ();
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
Phy80211::end_rx (Packet *packet, RxEvent *event)
{
	assert (is_state_rx ());
	assert (event->get_end_time_us () == now_us ());
	assert (m_end_rx_event != 0);
	m_end_rx_event = 0;

	NiChanges ni;
	double noise_interference_w = calculate_noise_interference_w (event, &ni);
	double snr = calculate_snr (event->get_rx_power_w (),
				    noise_interference_w,
				    get_mode (event->get_payload_mode ()));

	/* calculate the SNIR at the start of the packet and accumulate
	 * all SNIR changes in the snir vector.
	 */
	double per = calculate_per (event, &ni);
	
	if (m_random->get_double () > per) {
		notify_rx_end (now_s (), true);
		switch_to_idle_from_sync ();
		(*m_rx_ok_callback) (packet, snr, event->get_payload_mode ());
	} else {
		/* failure. */
		notify_rx_end (now_s (), false);
		switch_to_idle_from_sync ();
		(*m_rx_error_callback) (packet);
	}
	packet->unref ();
}




}; // namespace yans
