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
#ifndef ARF_MAC_STATIONS_H
#define ARF_MAC_STATIONS_H

#include "mac-stations.h"
#include "mac-station.h"

namespace yans {

class ArfMacStations : public MacStations {
public:
	ArfMacStations (uint8_t n_modes);
	virtual ~ArfMacStations ();

	uint8_t get_n_modes (void) const;
private:
	virtual class MacStation *create_station (void);
	uint8_t m_n_modes;
};


class ArfMacStation : public MacStation
{
public:
	ArfMacStation (ArfMacStations *stations,
		       int min_timer_timeout,
		       int min_success_threshold);
	virtual ~ArfMacStation ();

	virtual void report_rx_ok (double rx_snr, int tx_mode);

	virtual void report_rts_failed (void);
	virtual void report_data_failed (void);
	virtual void report_rts_ok (double cts_snr, int cts_mode);
	virtual void report_data_ok (double ack_snr, int ack_mode);
	virtual void report_final_rts_failed (void);
	virtual void report_final_data_failed (void);
	virtual int get_data_mode (int size);
	virtual int get_rts_mode (void);

private:
	ArfMacStations *m_stations;

	int m_timer;
	int m_success;
	int m_failed;
	bool m_recovery;
	int m_retry;
	
	int m_timer_timeout;
	int m_success_threshold;

	int m_rate;
	
	int m_min_timer_timeout;
	int m_min_success_threshold;
	
private:
	virtual void report_recovery_failure (void);
	virtual void report_failure (void);

	int get_max_rate (void);
	int get_min_rate (void);

	bool need_recovery_fallback (void);
	bool need_normal_fallback (void);
	
protected:
	int get_min_timer_timeout (void);
	int get_min_success_threshold (void);
	
	int get_timer_timeout (void);
	int get_success_threshold (void);
	
	void set_timer_timeout (int timer_timeout);
	void set_success_threshold (int success_threshold);
};

}; // namespace yans

#endif /* ARF_MAC_STATIONS_H */
