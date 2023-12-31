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

#ifndef NETWORK_INTERFACE_80211_SIMPLE_FACTORY_H
#define NETWORK_INTERFACE_80211_SIMPLE_FACTORY_H

#include <stdint.h>
#include "yans/mac-address.h"

namespace yans {

class Position;
class NetworkInterface80211Simple;

class NetworkInterface80211SimpleFactory {
public:
	NetworkInterface80211SimpleFactory ();
	~NetworkInterface80211SimpleFactory ();

	void set_arf (void);
	void set_aarf (void);
	void set_cr (uint8_t data_mode, uint8_t ctl_mode);
	void set_ideal (double ber);

	/* absolute reception threshold. dBm. */
	void set_phy_ed_threshold_dbm (double dbm);
	/* Ratio of energy lost by receiver. dB. */
	void set_phy_rx_noise_db (double rx_noise);
	/* absolute transmission energy. dBm. */
	void set_phy_tx_power_dbm (double tx_power_base,
				   double tx_power_end,
				   uint8_t n_tx_power);

	/* no unit. */
	void set_prop_system_loss  (double system_loss);
	void set_prop_tx_gain_dbm (double tx_gain);
	void set_prop_rx_gain_dbm (double rx_gain);
	void set_prop_frequency_hz (double frequency);

	void set_rts_cts_threshold (uint32_t size);

	NetworkInterface80211Simple *create (MacAddress address, Position *position);
private:
	enum {
		RATE_ARF,
		RATE_AARF,
		RATE_CR,
		RATE_IDEAL
	} m_rate_control_mode;

	uint8_t m_cr_data_mode;
	uint8_t m_cr_ctl_mode;

	double m_ideal_ber;

	double m_phy_ed_threshold_dbm;
	double m_phy_rx_noise_db;
	double m_phy_tx_power_base_dbm;
	double m_phy_tx_power_end_dbm;
	uint8_t m_phy_n_tx_power;

	double m_prop_system_loss;
	double m_prop_tx_gain_dbm;
	double m_prop_rx_gain_dbm;
	double m_prop_frequency_hz;

	uint32_t m_rts_cts_threshold;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_SIMPLE_FACTORY_H */
