/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "ideal-mac-stations.h"
#include "phy-80211.h"
#include <math.h>

#define noIDEAL_DEBUG 1

#ifdef IDEAL_DEBUG
#include <iostream>
#  define TRACE(x) \
std::cout << "IDEAL TRACE " << x << std::endl;
#else
#  define TRACE(x)
#endif


namespace yans {

IdealMacStations::IdealMacStations ()
{}
IdealMacStations::~IdealMacStations ()
{}

MacStation *
IdealMacStations::create_station (void)
{
  return new IdealMacStation (this);
}

double 
IdealMacStations::ratio_to_db (double ratio) const
{
	return 10 * log10 (ratio);
}

double 
IdealMacStations::db_to_ratio (double db) const
{
	return pow (10, db / 10);
}

uint8_t 
IdealMacStations::snr_to_snr (double snr) const
{
	if (snr > m_max_snr) {
		TRACE ("snr: "<<snr<<"->"<<255);
		return 255;
	}
	if (snr < m_min_snr) {
		TRACE ("snr: "<<snr<<"->"<<0);
		return 0;
	}
	double ratio = 255 * (snr - m_min_snr) / (m_max_snr - m_min_snr);
	TRACE ("snr: "<<snr<<"->"<<ratio);
	return (uint8_t) ratio;
}
double 
IdealMacStations::snr_to_snr (uint8_t snr) const
{
	double d = m_min_snr + (m_max_snr - m_min_snr) * snr / 255;
	TRACE ("snr back: "<<(uint32_t)snr<<"->"<<d);
	return d;
}
uint8_t 
IdealMacStations::get_mode (uint8_t snr) const
{
	double d_snr = snr_to_snr (snr);
	if (d_snr < (*m_thresholds.begin ())) {
		return 0;
	}
	uint8_t mode = 0;
	for (ThresholdsI i = m_thresholds.begin (); i != m_thresholds.end (); i++) {
		if (d_snr < (*i)) {
			TRACE ("use mode="<<(uint32_t)(mode-1)<<", snr="<<d_snr);
			return mode - 1;
		}
		mode++;
	}
	mode = m_thresholds.size () - 1;
	TRACE ("use mode="<<(uint32_t)mode<<", snr="<<d_snr);
	return mode;
}
void 
IdealMacStations::initialize_thresholds (Phy80211 const *phy, double ber)
{
	uint8_t n_modes = phy->get_n_modes ();
	for (uint8_t i = 0; i < n_modes; i++) {
		m_thresholds.push_back (phy->calculate_snr (i, ber));
	}
	uint32_t k = 0;
	m_min_snr = 0;
	m_max_snr = 1e-15;
	for (ThresholdsI j = m_thresholds.begin (); j != m_thresholds.end (); j++) {
		TRACE ("mode="<<k<<", threshold="<<(*j));
		if ((*j) > m_max_snr) {
			m_max_snr = (*j);
		}
		k++;
	}
	TRACE ("max snr="<<m_max_snr<<", min snr="<<m_min_snr);
}



IdealMacStation::IdealMacStation (IdealMacStations *stations)
	: m_stations (stations)
{}
IdealMacStation::~IdealMacStation ()
{}
void 
IdealMacStation::report_rx_ok (double rx_snr, uint8_t tx_mode)
{}
void 
IdealMacStation::report_rts_failed (void)
{}
void 
IdealMacStation::report_data_failed (void)
{}
void 
IdealMacStation::report_rts_ok (double cts_snr, uint8_t cts_mode, uint8_t rts_snr)
{
	TRACE ("got cts for rts snr="<<(uint32_t)rts_snr);
	m_last_snr = rts_snr;
}
void 
IdealMacStation::report_data_ok (double ack_snr, uint8_t ack_mode, uint8_t data_snr)
{
	TRACE ("got cts for rts snr="<<(uint32_t)data_snr);
	m_last_snr = data_snr;
}
uint8_t 
IdealMacStation::get_data_mode (int size)
{
	return m_stations->get_mode (m_last_snr);
}
uint8_t 
IdealMacStation::get_rts_mode (void)
{
	return 0;
}
uint8_t 
IdealMacStation::snr_to_snr (double snr)
{
	return m_stations->snr_to_snr (snr);
}

}; // namespace yans
