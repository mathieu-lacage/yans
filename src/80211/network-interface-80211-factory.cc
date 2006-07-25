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
#include "mac-stations.h"
#include "arf-mac-stations.h"
#include "aarf-mac-stations.h"
#include "cr-mac-stations.h"
#include "ideal-mac-stations.h"
#include "mac-low.h"
#include "mac-parameters.h"
#include "mac-tx-middle.h"
#include "mac-rx-middle.h"
#include "dca-txop.h"
#include "mac-high-adhoc.h"
#include "mac-high-nqsta.h"
#include "mac-high-nqap.h"
#include "supported-rates.h"

namespace yans {

NetworkInterface80211Factory::NetworkInterface80211Factory ()
	: m_rate_control_mode (RATE_ARF),
	  m_phy_ed_threshold_dbm (-140),
	  m_phy_rx_noise_db (7),
	  m_phy_tx_power_base_dbm (16.0206),
	  m_phy_tx_power_end_dbm (16.0206),
	  m_phy_n_tx_power (1),
	  m_prop_system_loss (1.0),
	  m_prop_tx_gain_dbm (1.0),
	  m_prop_rx_gain_dbm (1.0),
	  m_prop_frequency_hz (5.150e9),
	  m_mac_rts_cts_threshold (2000),
	  m_mac_fragmentation_threshold (2000),
	  m_mac_max_ssrc (7),
	  m_mac_max_slrc (7)
{}
NetworkInterface80211Factory::~NetworkInterface80211Factory ()
{}

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
NetworkInterface80211Factory::set_ideal (double ber)
{
	m_rate_control_mode = RATE_IDEAL;
	m_ideal_ber = ber;
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

void 
NetworkInterface80211Factory::set_mac_rts_cts_threshold (uint32_t size)
{
	m_mac_rts_cts_threshold = size;
}
void 
NetworkInterface80211Factory::set_mac_fragmentation_threshold (uint32_t size)
{
	m_mac_fragmentation_threshold = size;
}
void 
NetworkInterface80211Factory::set_mac_max_ssrc (uint32_t ssrc)
{
	m_mac_max_ssrc = ssrc;
}
void 
NetworkInterface80211Factory::set_mac_max_slrc (uint32_t slrc)
{
	m_mac_max_slrc = slrc;
}

void
NetworkInterface80211Factory::initialize_interface (NetworkInterface80211 *interface, Position *position) const
{
	PropagationModel *propagation = new PropagationModel ();
	propagation->set_tx_gain_dbm (m_prop_tx_gain_dbm);
	propagation->set_rx_gain_dbm (m_prop_rx_gain_dbm);
	propagation->set_system_loss (m_prop_system_loss);
	propagation->set_frequency_hz (m_prop_frequency_hz);
	propagation->set_position (position);
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

	MacParameters *parameters = new MacParameters ();
	parameters->initialize_80211a (phy);
	parameters->set_rts_cts_threshold (m_mac_rts_cts_threshold);
	parameters->set_fragmentation_threshold (m_mac_fragmentation_threshold);
	parameters->set_max_ssrc (m_mac_max_ssrc);
	parameters->set_max_slrc (m_mac_max_slrc);
	interface->m_parameters = parameters;

	MacLow *low = new MacLow ();
	low->set_interface (interface);
	low->set_phy (phy);
	low->set_stations (stations);
	low->set_parameters (parameters);
	phy->set_receive_ok_callback (make_callback (&MacLow::receive_ok, low));
	phy->set_receive_error_callback (make_callback (&MacLow::receive_error, low));
	interface->m_low = low;

	MacRxMiddle *rx_middle = new MacRxMiddle ();
	low->set_rx_callback (make_callback (&MacRxMiddle::receive, rx_middle));
	//rx_middle->set_callback (make_callback ());
	interface->m_rx_middle = rx_middle;

	MacTxMiddle *tx_middle = new MacTxMiddle ();
	interface->m_tx_middle = tx_middle;
}

DcaTxop *
NetworkInterface80211Factory::create_dca (NetworkInterface80211 const*interface) const
{
	DcaTxop *dca = new DcaTxop ();
	dca->set_parameters (interface->m_parameters);
	dca->set_tx_middle (interface->m_tx_middle);
	dca->set_low (interface->m_low);
	dca->set_phy (interface->m_phy);
	// 802.11a
	uint64_t difs = interface->m_parameters->get_sifs_us () + 
		2 * interface->m_parameters->get_slot_time_us ();
	uint64_t eifs = difs + interface->m_parameters->get_sifs_us () + 
		interface->m_phy->calculate_tx_duration_us (2+2+6+4, 0);
	dca->set_difs_us (difs);
	dca->set_eifs_us (eifs);
	dca->set_cw_bounds (15, 1023);
	dca->set_max_queue_size (400);
	dca->set_max_queue_delay_us (10000000); // 10s
	return dca;
}


NetworkInterface80211Adhoc *
NetworkInterface80211Factory::create_adhoc (MacAddress address, Position *position)
{
	NetworkInterface80211Adhoc *interface = new NetworkInterface80211Adhoc (address);
	interface->m_ssid = m_ssid;

	initialize_interface (interface, position);

	DcaTxop *dca = create_dca (interface);
	interface->m_dca = dca;


	MacHighAdhoc *high = new MacHighAdhoc ();
	high->set_interface (interface);
	high->set_dca_txop (dca);
	high->set_forward_callback (make_callback (&NetworkInterface80211::forward_up_data, 
						   static_cast<NetworkInterface80211 *> (interface)));
	interface->m_rx_middle->set_forward_callback (make_callback (&MacHighAdhoc::receive, high));
	interface->m_high = high;

	return interface;
}

NetworkInterface80211Nqsta *
NetworkInterface80211Factory::create_nqsta (MacAddress address, Position *position)
{
	NetworkInterface80211Nqsta *interface = new NetworkInterface80211Nqsta (address);
	interface->m_ssid = m_ssid;

	initialize_interface (interface, position);

	DcaTxop *dca = create_dca (interface);
	interface->m_dca = dca;

	SupportedRates rates;
	for (uint32_t mode = 0; mode < interface->m_phy->get_n_modes (); mode++) {
		rates.add_supported_rate (interface->m_phy->get_mode_bit_rate (mode));
	}

	MacHighNqsta *high = new MacHighNqsta ();
	high->set_interface (interface);
	high->set_dca_txop (dca);
	high->set_forward_callback (make_callback (&NetworkInterface80211::forward_up_data, 
						   static_cast<NetworkInterface80211 *> (interface)));
	high->set_associated_callback (make_callback (&NetworkInterface80211Nqsta::associated, interface));
	high->set_supported_rates (rates);
	interface->m_rx_middle->set_forward_callback (make_callback (&MacHighNqsta::receive, high));
	interface->m_high = high;

	return interface;
}

NetworkInterface80211Nqap *
NetworkInterface80211Factory::create_nqap (MacAddress address, Position *position)
{
	NetworkInterface80211Nqap *interface = new NetworkInterface80211Nqap (address);
	interface->m_ssid = m_ssid;

	initialize_interface (interface, position);

	DcaTxop *dca = create_dca (interface);
	interface->m_dca = dca;

	SupportedRates rates;
	for (uint32_t mode = 0; mode < interface->m_phy->get_n_modes (); mode++) {
		rates.add_supported_rate (interface->m_phy->get_mode_bit_rate (mode));
	}

	MacHighNqap *high = new MacHighNqap ();
	high->set_interface (interface);
	high->set_dca_txop (dca);
	high->set_stations (interface->m_stations);
	high->set_forward_callback (make_callback (&NetworkInterface80211::forward_up_data, 
						   static_cast<NetworkInterface80211 *> (interface)));
	high->set_supported_rates (rates);
	interface->m_rx_middle->set_forward_callback (make_callback (&MacHighNqap::receive, high));
	interface->m_high = high;

	return interface;
}


}; // namespace yans

