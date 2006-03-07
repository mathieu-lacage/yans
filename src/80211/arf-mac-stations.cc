/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2004,2005,2006 INRIA
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

#include "mac-stations.h"
#include "mac-station.h"
#include "arf-mac-stations.h"

#include <cassert>

namespace yans {
	
ArfMacStation::ArfMacStation (ArfMacStations *stations,
			      int min_timer_timeout,
			      int min_success_threshold)
{
	m_stations = stations;
        m_min_timer_timeout = min_timer_timeout;
        m_min_success_threshold = min_success_threshold;
        m_success_threshold = m_min_success_threshold;
        m_timer_timeout = m_min_timer_timeout;
        m_rate = get_min_rate ();

        m_success = 0;
        m_failed = 0;
        m_recovery = false;
        m_retry = 0;
        m_timer = 0;
}
ArfMacStation::~ArfMacStation ()
{}

int 
ArfMacStation::get_max_rate (void)
{
	return m_stations->get_n_modes ();
}
int 
ArfMacStation::get_min_rate (void)
{
	return 0;
}

bool 
ArfMacStation::need_recovery_fallback (void)
{
	if (m_retry == 1) {
		return true;
	} else {
		return false;
	}
}
bool 
ArfMacStation::need_normal_fallback (void)
{
	int retry_mod = (m_retry - 1) % 2;
	if (retry_mod == 1) {
		return true;
	} else {
		return false;
	}
}



void 
ArfMacStation::report_rts_failed (void)
{}
/**
 * It is important to realize that "recovery" mode starts after failure of
 * the first transmission after a rate increase and ends at the first successful
 * transmission. Specifically, recovery mode transcends retransmissions boundaries.
 * Fundamentally, ARF handles each data transmission independently, whether it
 * is the initial transmission of a packet or the retransmission of a packet.
 * The fundamental reason for this is that there is a backoff between each data
 * transmission, be it an initial transmission or a retransmission.
 */
void 
ArfMacStation::report_data_failed (void)
{
        m_timer++;
        m_failed++;
        m_retry++;
        m_success = 0;

        if (m_recovery) {
                assert (m_retry >= 1);
                if (need_recovery_fallback ()) {
                        report_recovery_failure ();
                        if (m_rate != get_min_rate ()) {
                                m_rate--;
                        }
                }
                m_timer = 0;
        } else {
                assert (m_retry >= 1);
                if (need_normal_fallback ()) {
                        report_failure ();
                        if (m_rate != get_min_rate ()) {
                                m_rate--;
                        }
                }
                if (m_retry >= 2) {
                        m_timer = 0;
                }
        }
}
void 
ArfMacStation::report_rx_ok (double rx_snr, uint8_t tx_mode)
{}
void ArfMacStation::report_rts_ok (double cts_snr, uint8_t cts_mode, uint8_t rts_snr)
{
	assert (rts_snr == 0);
}
void ArfMacStation::report_data_ok (double ack_snr, uint8_t ack_mode, uint8_t data_snr)
{
	assert (data_snr == 0);
	m_timer++;
        m_success++;
        m_failed = 0;
        m_recovery = false;
        m_retry = 0;
        if ((m_success == get_success_threshold () ||
             m_timer == get_timer_timeout ()) &&
            (m_rate < (get_max_rate () - 1))) {
                m_rate++;
                m_timer = 0;
                m_success = 0;
                m_recovery = true;
        }

}
uint8_t 
ArfMacStation::snr_to_snr (double snr)
{
	return 0;
}
void ArfMacStation::report_final_rts_failed (void)
{}
void ArfMacStation::report_final_data_failed (void)
{}
uint8_t 
ArfMacStation::get_data_mode (int size)
{
	return m_rate;
}
uint8_t 
ArfMacStation::get_rts_mode (void)
{
	return 0;
}

void ArfMacStation::report_recovery_failure (void)
{}
void ArfMacStation::report_failure (void)
{}
int ArfMacStation::get_min_timer_timeout (void)
{
        return m_min_timer_timeout;
}
int ArfMacStation::get_min_success_threshold (void)
{
        return m_min_success_threshold;
}
int ArfMacStation::get_timer_timeout (void)
{
        return m_timer_timeout;
}
int ArfMacStation::get_success_threshold (void)
{
        return m_success_threshold;
}
void ArfMacStation::set_timer_timeout (int timer_timeout)
{
        assert (timer_timeout >= m_min_timer_timeout);
        m_timer_timeout = timer_timeout;
}
void ArfMacStation::set_success_threshold (int success_threshold)
{
        assert (success_threshold >= m_min_success_threshold);
        m_success_threshold = success_threshold;
}





ArfMacStations::ArfMacStations (uint8_t n_modes)
	: m_n_modes (n_modes)
{}
ArfMacStations::~ArfMacStations ()
{}
MacStation *
ArfMacStations::create_station (void)
{
	/* XXX: use mac to access user and PHY params. */
	return new ArfMacStation (this, 15, 10);
}

uint8_t
ArfMacStations::get_n_modes (void) const
{
	return m_n_modes;
}


}; // namespace yans
