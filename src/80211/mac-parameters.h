/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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
#ifndef MAC_PARAMETERS_H
#define MAC_PARAMETERS_H

#include <stdint.h>

namespace yans {

class Phy80211;

class MacParameters {
public:
	MacParameters ();
	
	void initialize_80211a (Phy80211 const*phy);
	void set_slot_time_us (uint64_t slot_time);
	void set_max_ssrc (uint32_t ssrc);
	void set_max_slrc (uint32_t ssrc);
	void set_rts_cts_threshold (uint32_t threshold);
	void set_fragmentation_threshold (uint32_t threshold);

	// XXX AP-specific
	uint64_t get_beacon_interval_us (void) const;

	uint64_t get_pifs_us (void) const;
	uint64_t get_sifs_us (void) const;
	uint64_t get_slot_time_us (void) const;

	uint32_t get_max_ssrc (void) const;
	uint32_t get_max_slrc (void) const;
	uint32_t get_rts_cts_threshold (void) const;
	uint32_t get_fragmentation_threshold (void) const;
	uint64_t get_cts_timeout_us (void) const;
	uint64_t get_ack_timeout_us (void) const;
	uint64_t get_msdu_lifetime_us (void) const;
	uint32_t get_max_queue_size (void) const;
	uint64_t get_max_propagation_delay_us (void) const;

	uint32_t get_max_msdu_size (void) const;
	double get_cap_limit (void) const;
	double get_min_edca_traffic_proportion (void) const;
private:
	uint64_t m_cts_timeout_us;
	uint64_t m_ack_timeout_us;
	uint64_t m_sifs_us;
	uint64_t m_pifs_us;
	uint64_t m_slot_us;
	uint32_t m_max_ssrc;
	uint32_t m_max_slrc;
	uint32_t m_rts_cts_threshold;
	uint32_t m_fragmentation_threshold;
	uint64_t m_max_propagation_delay_us;
	static const double SPEED_OF_LIGHT; // m/s
};

}; // namespace yans

#endif /* MAC_PARAMETERS_H */
