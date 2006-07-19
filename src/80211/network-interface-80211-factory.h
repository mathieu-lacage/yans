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

#ifndef NETWORK_INTERFACE_80211_FACTORY_H
#define NETWORK_INTERFACE_80211_FACTORY_H

#include <stdint.h>
#include "ssid.h"
#include "mac-address.h"

namespace yans {

class NetworkInterface80211;
class NetworkInterface80211Adhoc;
class NetworkInterface80211Nqsta;
class NetworkInterface80211Nqap;
class DcaTxop;
class Position;

class NetworkInterface80211Factory {
public:
	NetworkInterface80211Factory ();
	~NetworkInterface80211Factory ();

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

	void set_mac_rts_cts_threshold (uint32_t size);
	void set_mac_fragmentation_threshold (uint32_t size);
	void set_mac_max_ssrc (uint32_t ssrc);
	void set_mac_max_slrc (uint32_t slrc);

	void set_ssid (Ssid ssid);

	NetworkInterface80211Adhoc *create_adhoc (MacAddress address, Position *position);
	NetworkInterface80211Nqsta *create_nqsta (MacAddress address, Position *position);
	NetworkInterface80211Nqap *create_nqap (MacAddress address, Position *position);
private:
	void initialize_interface (NetworkInterface80211 *interface, Position *position) const;
	DcaTxop *create_dca (NetworkInterface80211 const*interface) const;
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

	uint64_t m_prop_tx_delay_us;
	uint64_t m_prop_rx_delay_us;
	double m_prop_system_loss;
	double m_prop_tx_gain_dbm;
	double m_prop_rx_gain_dbm;
	double m_prop_frequency_hz;

	uint32_t m_mac_rts_cts_threshold;
	uint32_t m_mac_fragmentation_threshold;
	uint32_t m_mac_max_ssrc;
	uint32_t m_mac_max_slrc;

	Ssid m_ssid;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_FACTORY_H */
