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
#include "network-interface-80211-simple-factory.h"
#include "network-interface-80211-simple.h"
#include "propagation-model.h"
#include "phy-80211.h"
#include "mac-stations.h"
#include "arf-mac-stations.h"
#include "aarf-mac-stations.h"
#include "cr-mac-stations.h"
#include "ideal-mac-stations.h"
#include "mac-simple.h"
#include "arp.h"

namespace yans {

NetworkInterface80211SimpleFactory::NetworkInterface80211SimpleFactory ()
	: m_rate_control_mode (RATE_ARF),
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
NetworkInterface80211SimpleFactory::~NetworkInterface80211SimpleFactory ()
{}

void 
NetworkInterface80211SimpleFactory::set_arf (void)
{
	m_rate_control_mode = RATE_ARF;
}
void 
NetworkInterface80211SimpleFactory::set_aarf (void)
{
	m_rate_control_mode = RATE_AARF;
}
void 
NetworkInterface80211SimpleFactory::set_cr (uint8_t data_mode, uint8_t ctl_mode)
{
	m_rate_control_mode = RATE_CR;
	m_cr_data_mode = data_mode;
	m_cr_ctl_mode = ctl_mode;
}
void 
NetworkInterface80211SimpleFactory::set_ideal (double ber)
{
	m_rate_control_mode = RATE_IDEAL;
	m_ideal_ber = ber;
}

void 
NetworkInterface80211SimpleFactory::set_phy_ed_threshold_dbm (double dbm)
{
	m_phy_ed_threshold_dbm = dbm;
}
void 
NetworkInterface80211SimpleFactory::set_phy_rx_noise_db (double rx_noise_db)
{
	m_phy_rx_noise_db = rx_noise_db;
}
void 
NetworkInterface80211SimpleFactory::set_phy_tx_power_dbm (double tx_power_base,
						    double tx_power_end,
						    uint8_t n_tx_power)
{
	m_phy_tx_power_base_dbm = tx_power_base;
	m_phy_tx_power_end_dbm = tx_power_end;
	m_phy_n_tx_power = n_tx_power;
}

void 
NetworkInterface80211SimpleFactory::set_prop_system_loss  (double system_loss)
{
	m_prop_system_loss = system_loss;
}
void 
NetworkInterface80211SimpleFactory::set_prop_tx_gain_dbm (double tx_gain)
{
	m_prop_tx_gain_dbm = tx_gain;
}
void 
NetworkInterface80211SimpleFactory::set_prop_rx_gain_dbm (double rx_gain)
{
	m_prop_rx_gain_dbm = rx_gain;
}
void 
NetworkInterface80211SimpleFactory::set_prop_frequency_hz (double frequency)
{
	m_prop_frequency_hz = frequency;
}

void 
NetworkInterface80211SimpleFactory::set_rts_cts_threshold (uint32_t size)
{
	m_rts_cts_threshold = size;
}

NetworkInterface80211Simple *
NetworkInterface80211SimpleFactory::create (Host *host)
{
	NetworkInterface80211Simple *interface = new NetworkInterface80211Simple ();

	PropagationModel *propagation = new PropagationModel ();
	propagation->set_tx_gain_dbm (m_prop_tx_gain_dbm);
	propagation->set_rx_gain_dbm (m_prop_rx_gain_dbm);
	propagation->set_system_loss (m_prop_system_loss);
	propagation->set_frequency_hz (m_prop_frequency_hz);
	propagation->set_host (host);
	interface->m_propagation = propagation;


	Phy80211 *phy = new Phy80211 ();
	phy->set_propagation_model (propagation);
	phy->set_ed_threshold_dbm (m_phy_ed_threshold_dbm);
	phy->set_rx_noise_db (m_phy_rx_noise_db);
	phy->set_tx_power_increments_dbm (m_phy_tx_power_base_dbm,
					  m_phy_tx_power_end_dbm,
					  m_phy_n_tx_power);
	phy->configure_80211a ();
	interface->m_phy = phy;
	propagation->set_receive_callback (make_callback (&Phy80211::receive_packet, phy));


	MacStations *stations;
	switch (m_rate_control_mode) {
	case RATE_ARF:
		stations = new ArfMacStations (phy->get_n_modes ());
		break;
	case RATE_AARF:
		stations = new AarfMacStations (phy->get_n_modes ());
		break;
	case RATE_CR:
		stations = new CrMacStations (m_cr_data_mode, m_cr_ctl_mode);
		break;
	case RATE_IDEAL: {
		IdealMacStations *ideal = new IdealMacStations ();
		ideal->initialize_thresholds (phy, m_ideal_ber);
		stations = ideal;
	} break;
	default:
		// NOTREACHED
		stations = 0;
		break;
	}
	interface->m_stations = stations;

	MacSimple *mac = new MacSimple ();
	phy->set_receive_ok_callback (make_callback (&MacSimple::receive_ok, mac));
	phy->set_receive_error_callback (make_callback (&MacSimple::receive_error, mac));
	mac->set_phy (phy);
	mac->set_stations (stations);
	mac->set_interface (interface);
	mac->set_rts_cts_threshold (m_rts_cts_threshold);
	interface->m_mac = mac;

	Arp *arp = new Arp (interface);
	mac->set_receiver (make_callback (&NetworkInterface80211Simple::forward_data_up, interface));
	arp->set_sender (make_callback (&NetworkInterface80211Simple::send_data, interface),
			 make_callback (&NetworkInterface80211Simple::send_arp, interface));
	interface->m_arp = arp;


	return interface;
}

}; // namespace yans

