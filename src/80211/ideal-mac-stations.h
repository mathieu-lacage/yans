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
#ifndef IDEAL_MAC_STATIONS_H
#define IDEAL_MAC_STATIONS_H

#include <stdint.h>
#include <vector>
#include "mac-station.h"
#include "mac-stations.h"

namespace yans {

class Phy80211;

class IdealMacStations : public MacStations {
public:
	IdealMacStations ();
	virtual ~IdealMacStations ();
	uint8_t snr_to_snr (double snr) const;
	uint8_t get_mode (uint8_t snr) const;
	void initialize_thresholds (Phy80211 const *phy, double ber);
private:
	virtual class MacStation *create_station (void);
	double snr_to_snr (uint8_t snr) const;
	double ratio_to_db (double ratio) const;
	double db_to_ratio (double db) const;

	typedef std::vector<double> Thresholds;
	typedef std::vector<double>::const_iterator ThresholdsI;

	Thresholds m_thresholds;
	double m_min_snr;
	double m_max_snr;
};

class IdealMacStation : public MacStation {
public:
	IdealMacStation (IdealMacStations *stations);

	virtual ~IdealMacStation ();

	virtual void report_rx_ok (double rx_snr, uint8_t tx_mode);
	virtual void report_rts_failed (void);
	virtual void report_data_failed (void);
	virtual void report_rts_ok (double cts_snr, uint8_t cts_mode, uint8_t rts_snr);
	virtual void report_data_ok (double ack_snr, uint8_t ack_mode, uint8_t data_snr);
	virtual uint8_t get_data_mode (int size);
	virtual uint8_t get_rts_mode (void);
	virtual uint8_t snr_to_snr (double snr);

private:
	IdealMacStations *m_stations;
	uint8_t m_last_snr;
};

}; // namespace yans

#endif /* MAC_STA_H */
