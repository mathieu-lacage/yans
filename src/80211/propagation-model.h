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
 * Authors: Hossein Manshaei, Mathieu Lacage
 */
#ifndef PROPAGATION_MODEL_H
#define PROPAGATION_MODEL_H

#include <stdint.h>
#include "callback.h"
#include "packet.h"

namespace yans {
/**
 * This propagation model implements a path-loss model
 * with a path loss exponent of 3 and uses 1.0m as its
 * reference distance. The reference power is calculated
 * with a Friis model. The propagation delay is calculated
 * with a simple free-space model.
 *
 * This is coherent with the models used by Gavin 
 * Holland in "A Rate-Adaptive MAC Protocol for Multi-Hop 
 * Wireless Networks"
 *
 * The tricky aspect of this code relates to the 
 * unit conversions.
 */

class Position;
class BaseChannel80211;

class PropagationModel {
public:
	typedef Callback<void,ConstPacketPtr, double, uint8_t, uint8_t> RxCallback;
	PropagationModel ();
	~PropagationModel ();

	void set_position (Position *position);
	void set_channel (BaseChannel80211 *channel);
	/* the unit of the power is Watt. */
	void set_receive_callback (RxCallback callback);

	void get_position (double &x, double &y, double &z) const;
	uint64_t get_tx_delay (void) const;
	uint64_t get_rx_delay (void) const;
	uint64_t get_prop_delay_us (double from_x, double from_y, double from_z) const;
	double get_rx_power_w (double tx_power_dbm, double from_x, double from_y, double from_z) const;


	/* tx power unit: dBm */
	void send (ConstPacketPtr packet, double tx_power_dbm, uint8_t tx_mode, uint8_t stuff) const;
	void receive (ConstPacketPtr packet, double rx_power_w,
		      uint8_t tx_mode, uint8_t stuff);

	void set_tx_delay (uint64_t tx_delay);
	void set_rx_delay (uint64_t rx_delay);
	/* unit: dBm */
	void set_tx_gain_dbm (double tx_gain);
	/* unit: dBm */
	void set_rx_gain_dbm (double rx_gain);
	/* no unit */
	void set_system_loss (double system_loss);
	/* unit: Hz */
	void set_frequency_hz (double frequency);
private:
	double dbm_to_w (double dbm) const;
	double db_to_w (double db) const;
	double get_lambda (void) const;
	double distance (double from_x, double from_y, double from_z) const;
	double get_rx_power_w (double tx_power_dbm, double distance) const;

	RxCallback m_rx_callback;
	double m_tx_gain_dbm;
	double m_rx_gain_dbm;
	double m_system_loss;
	double m_lambda;
	uint64_t m_tx_delay;
	uint64_t m_rx_delay;
	Position *m_position;
	BaseChannel80211 *m_channel;
	static const double PI;
	static const double SPEED_OF_LIGHT;
};

}; // namespace yans

#endif /* PROPAGATION_MODEL_H */
