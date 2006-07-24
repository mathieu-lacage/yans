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

#ifndef PHY_80211_H
#define PHY_80211_H

#include <vector>
#include <list>
#include <stdint.h>
#include "callback.h"
#include "count-ptr-holder.tcc"
#include "event.h"
#include "packet.h"
#include "callback-logger.h"


namespace yans {

class TransmissionMode;
class PropagationModel;
class RandomUniform;
class RxEvent;
class TraceContainer;

class Phy80211Listener {
public:
	virtual ~Phy80211Listener ();

	/* we have received the first bit of a packet. We decided
	 * that we could synchronize on this packet. It does not mean
	 * we will be able to successfully receive completely the
	 * whole packet. It means we will report a BUSY status.
	 * rxEnd will be invoked later to report whether or not
	 * the packet was successfully received.
	 */
	virtual void notify_rx_start (uint64_t duration_us) = 0;
	/* we have received the last bit of a packet for which
	 * rxStart was invoked first. 
	 */
	virtual void notify_rx_end_ok (void) = 0;
	virtual void notify_rx_end_error (void) = 0;
	/* we start the transmission of a packet.
	 */
	virtual void notify_tx_start (uint64_t duration_us) = 0;
	virtual void notify_cca_busy_start (uint64_t duration_us) = 0;
};



class Phy80211
{
public:
	typedef Callback<void,ConstPacketPtr , double, uint8_t, uint8_t> SyncOkCallback;
	typedef Callback<void,ConstPacketPtr , double> SyncErrorCallback;

	Phy80211 ();
	virtual ~Phy80211 ();

	void register_traces (TraceContainer *container);

	void set_propagation_model (PropagationModel *propagation);
	void set_receive_ok_callback (SyncOkCallback callback);
	void set_receive_error_callback (SyncErrorCallback callback);

	/* rx_power unit is Watt */
	void receive_packet (ConstPacketPtr packet,
			     double rx_power_w,
			     uint8_t tx_mode,
			     uint8_t stuff);
	void send_packet (ConstPacketPtr packet, uint8_t tx_mode, uint8_t tx_power, uint8_t stuff);

	void register_listener (Phy80211Listener *listener);

	bool is_state_cca_busy (void);
	bool is_state_idle (void);
	bool is_state_busy (void);
	bool is_state_sync (void);
	bool is_state_tx (void);
	uint64_t get_state_duration_us (void);
	uint64_t get_delay_until_idle_us (void);

	uint64_t calculate_tx_duration_us (uint32_t size, uint8_t payload_mode) const;

	void configure_80211a (void);
	void set_ed_threshold_dbm (double rx_threshold);
	void set_rx_noise_db (double rx_noise);	
	void set_tx_power_increments_dbm (double tx_power_base, 
				      double tx_power_end, 
				      int n_tx_power);
	uint32_t get_n_modes (void) const;
	uint32_t get_mode_bit_rate (uint8_t mode) const;
	uint32_t get_n_txpower (void) const;
	/* return snr: W/W */
	double calculate_snr (uint8_t tx_mode, double ber) const;

private:
	enum Phy80211State {
		SYNC,
		TX,
		CCA_BUSY,
		IDLE
	};
	class NiChange {
	public:
		NiChange (uint64_t time, double delta);
		uint64_t get_time_us (void) const;
		double get_delta (void) const;
		bool operator < (NiChange const &o) const;
	private:
		uint64_t m_time;
		double m_delta;
	};
	typedef std::vector<TransmissionMode *> Modes;
	typedef std::vector<TransmissionMode *>::const_iterator ModesCI;
	typedef std::list<Phy80211Listener *> Listeners;
	typedef std::list<Phy80211Listener *>::const_iterator ListenersCI;
	typedef std::list<RxEvent *> Events;
	typedef std::list<RxEvent *>::iterator EventsI;
	typedef std::list<RxEvent *>::const_iterator EventsCI;
	typedef std::vector <NiChange> NiChanges;
	typedef std::vector <NiChange>::iterator NiChangesI;

private:	
	char const *state_to_string (enum Phy80211State state);
	enum Phy80211State get_state (void);
	double get_ed_threshold_w (void) const;
	double dbm_to_w (double dbm) const;
	double db_to_ratio (double db) const;
	uint64_t now_us (void) const;
	uint64_t get_max_packet_duration_us (void) const;
	void add_tx_rx_mode (TransmissionMode *mode);
	void cancel_rx (void);
	TransmissionMode *get_mode (uint8_t tx_mode) const;
	double get_power_dbm (uint8_t power) const;
	void notify_tx_start (uint64_t duration_us);
	void notify_wakeup (void);
	void notify_sync_start (uint64_t duration_us);
	void notify_sync_end_ok (void);
	void notify_sync_end_error (void);
	void notify_cca_busy_start (uint64_t duration_us);
	void switch_to_tx (uint64_t tx_duration_us);
	void switch_to_sync (uint64_t sync_duration_us);
	void switch_from_sync (void);
	void switch_to_cca_busy (uint64_t duration_us);
	void append_event (RxEvent *event);
	double calculate_noise_interference_w (RxEvent *event, NiChanges *ni) const;
	double calculate_snr (double signal, double noise_interference, TransmissionMode *mode) const;
	double calculate_chunk_success_rate (double snir, uint64_t delay, TransmissionMode *mode) const;
	double calculate_per (RxEvent const*event, NiChanges *ni) const;
	void end_sync (ConstPacketPtr packet, CountPtrHolder<RxEvent> event, uint8_t stuff);
	double get_snr_for_ber (TransmissionMode *mode, double ber) const;
private:
	uint64_t m_tx_prepare_delay_us;
	uint64_t m_plcp_preamble_delay_us;
	uint32_t m_plcp_header_length;
	uint64_t m_max_packet_duration_us;

	double   m_ed_threshold_w; /* unit: W */
	double   m_rx_noise_ratio;
	double   m_tx_power_base_dbm;
	double   m_tx_power_end_dbm;
	uint32_t m_n_tx_power;

	
	bool m_syncing;
	uint64_t m_end_tx_us;
	uint64_t m_end_sync_us;
	uint64_t m_end_cca_busy_us;
	uint64_t m_previous_state_change_time_us;

	PropagationModel *m_propagation;
	SyncOkCallback m_sync_ok_callback;
	SyncErrorCallback m_sync_error_callback;
	Modes m_modes;
	Listeners m_listeners;
	Event m_end_sync_event;
	Events m_events;
	RandomUniform *m_random;
	/* param1: - true: sync completed ok
	 *         - false: sync completed with failure
	 * Invoked when the last bit of a signal (which was
	 * synchronized upon) is received.
	 * Reports whether or not the signal was received
	 * successfully.
	 */
	CallbackLogger<bool> m_end_sync_logger;
	/* param1: duration (us)
	 * param2: signal energy (w)
	 * Invoked whenever the first bit of a signal is received.
	 */
	CallbackLogger<uint64_t,double> m_start_rx_logger;
	/* param1: duration (us)
	 * param2: signal energy (w)
	 * Invoked whenever the first bit of a signal is 
	 * synchronized upon.
	 */
	CallbackLogger<uint64_t,double> m_start_sync_logger;
	/* param1: duration (us)
	 * param2: tx mode (bit rate: bit/s)
	 * param3: tx power (dbm)
	 * Invoked whenever we send the first bit of a signal.
	 */
	CallbackLogger<uint64_t, uint32_t, double> m_start_tx_logger;
	/* 80211-phy-state
	 * param1: start (us)
	 * param2: duration (us)
	 * param3: state: 0 -> TX, 1 -> SYNC, 2 -> CCA, 3 -> IDLE
	 */
	CallbackLogger<uint64_t,uint64_t,uint8_t> m_state_logger;
};

}; // namespace yans


#endif /* PHY_80211_H */
