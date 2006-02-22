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

namespace yans {

class Ssid {
public:
	Ssid ();
	Ssid (char const ssid[14]);
private:
	char m_ssid[14];
};

class NetworkInterface80211;

class NetworkInterface80211Factory {
public:
	NetworkInterface80211Factory ();
	~NetworkInterface80211Factory ();

	void set_ssid (Ssid const ssid);

	void set_qap (void);
	void set_qsta (void);
	void set_nqap (void);
	void set_nqsta (void);
	void set_adhoc (void);

	void set_arf (void);
	void set_aarf (void);
	void set_cr (uint8_t data_mode, uint8_t ctl_mode);

	void set_80211a (void);

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

	NetworkInterface80211 *create (void);
private:
	enum {
		MODE_QAP,
		MODE_NQAP,
		MODE_QSTA,
		MODE_NQSTA,
		MODE_ADHOC
	} m_mac_mode;

	enum {
		STANDARD_80211_A
		/* This is not really implemented but it should be
		   trivial to do.
		   STANDARD_80211_MODE_B 
		*/
	} m_standard;

	enum {
		RATE_ARF,
		RATE_AARF,
		RATE_CR
	} m_rate_control_mode;

	Ssid m_ssid;

	uint8_t m_cr_data_mode;
	uint8_t m_cr_ctl_mode;

	double m_phy_ed_threshold_dbm;
	double m_phy_rx_noise_db;
	double m_phy_tx_power_base_dbm;
	double m_phy_tx_power_end_dbm;
	uint8_t m_phy_n_tx_power;

	double m_prop_system_loss;
	double m_prop_tx_gain_dbm;
	double m_prop_rx_gain_dbm;
	double m_prop_frequency_hz;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_80211_FACTORY_H */
