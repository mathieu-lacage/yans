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
#include "network-interface-80211-factory.h"
#include "network-interface-80211.h"
#include "propagation-model.h"
#include "phy-80211.h"


namespace yans {

Ssid::Ssid ()
{}

Ssid::Ssid (char const ssid[14])
{
	memcpy (m_ssid, ssid, 14);
}



NetworkInterface80211Factory::NetworkInterface80211Factory ()
	: m_mac_mode (MODE_ADHOC),
	  m_standard (STANDARD_80211_A),
	  m_rate_control_mode (RATE_ARF),
	  m_ssid ("default-ssid "),
	  m_phy_ed_threshold_dbm (-140),
	  m_phy_rx_noise_db (7),
	  m_phy_tx_power_base_dbm (16.0206),
	  m_phy_tx_power_end_dbm (16.0206),
	  m_phy_n_tx_power (1),
	  m_prop_system_loss (1.0),
	  m_prop_tx_gain_dbm (1.0),
	  m_prop_rx_gain_dbm (1.0),
	  m_prop_frequency_hz (5.150e9)
	  
{}
NetworkInterface80211Factory::~NetworkInterface80211Factory ()
{}

void 
NetworkInterface80211Factory::set_ssid (Ssid const ssid)
{
	m_ssid = ssid;
}

void 
NetworkInterface80211Factory::set_qap (void)
{
	m_mac_mode = MODE_QAP;
}
void 
NetworkInterface80211Factory::set_qsta (void)
{
	m_mac_mode = MODE_QSTA;
}
void 
NetworkInterface80211Factory::set_nqap (void)
{
	m_mac_mode = MODE_NQAP;
}
void 
NetworkInterface80211Factory::set_nqsta (void)
{
	m_mac_mode = MODE_NQSTA;
}
void 
NetworkInterface80211Factory::set_adhoc (void)
{
	m_mac_mode = MODE_ADHOC;
}

void 
NetworkInterface80211Factory::set_arf (void)
{
	m_rate_control_mode = RATE_ARF;
}
void 
NetworkInterface80211Factory::set_aarf (void)
{
	m_rate_control_mode = RATE_AARF;
}
void 
NetworkInterface80211Factory::set_cr (uint8_t data_mode, uint8_t ctl_mode)
{
	m_rate_control_mode = RATE_CR;
	m_cr_data_mode = data_mode;
	m_cr_ctl_mode = ctl_mode;
}

void 
NetworkInterface80211Factory::set_80211a (void)
{
	m_standard = STANDARD_80211_A;
}

void 
NetworkInterface80211Factory::set_phy_ed_threshold_dbm (double dbm)
{
	m_phy_ed_threshold_dbm = dbm;
}
void 
NetworkInterface80211Factory::set_phy_rx_noise_db (double rx_noise_db)
{
	m_phy_rx_noise_db = rx_noise_db;
}
void 
NetworkInterface80211Factory::set_phy_tx_power_dbm (double tx_power_base,
						    double tx_power_end,
						    uint8_t n_tx_power)
{
	m_phy_tx_power_base_dbm = tx_power_base;
	m_phy_tx_power_end_dbm = tx_power_end;
	m_phy_n_tx_power = n_tx_power;
}

void 
NetworkInterface80211Factory::set_prop_system_loss  (double system_loss)
{
	m_prop_system_loss = system_loss;
}
void 
NetworkInterface80211Factory::set_prop_tx_gain_dbm (double tx_gain)
{
	m_prop_tx_gain_dbm = tx_gain;
}
void 
NetworkInterface80211Factory::set_prop_rx_gain_dbm (double rx_gain)
{
	m_prop_rx_gain_dbm = rx_gain;
}
void 
NetworkInterface80211Factory::set_prop_frequency_hz (double frequency)
{
	m_prop_frequency_hz = frequency;
}

NetworkInterface80211 *
NetworkInterface80211Factory::create (void)
{
	NetworkInterface80211 *interface = new NetworkInterface80211 ();

	PropagationModel *propagation = new PropagationModel ();
	propagation->set_tx_gain_dbm (m_prop_tx_gain_dbm);
	propagation->set_rx_gain_dbm (m_prop_rx_gain_dbm);
	propagation->set_system_loss (m_prop_system_loss);
	propagation->set_frequency_hz (m_prop_frequency_hz);


	Phy80211 *phy = new Phy80211 ();
	phy->set_propagation_model (propagation);
	phy->set_ed_threshold_dbm (m_phy_ed_threshold_dbm);
	phy->set_rx_noise_db (m_phy_rx_noise_db);
	phy->set_tx_power_increments_dbm (m_phy_tx_power_base_dbm,
					  m_phy_tx_power_end_dbm,
					  m_phy_n_tx_power);
	//phy->set_receive_ok_callback (make_callback ());
	//phy->set_receive_error_callback (make_callback ());

	return interface;
}

}; // namespace yans

