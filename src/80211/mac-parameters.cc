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

#include <cassert>

#include "mac-parameters.h"
#include "phy-80211.h"
#include "chunk-mac-80211-hdr.h"

#define PARAM_DEBUG 1

#ifdef PARAM_DEBUG
#  include <iostream>
#  define DEBUG(x) \
   std::cout << "PARAM " << x << std::endl;
#else /* MAC_DEBUG */
# define DEBUG(x)
#endif /* MAC_DEBUG */

namespace yans {

MacParameters::MacParameters ()
{
	m_rts_cts_threshold = 1000;
	m_fragmentation_threshold = 2000;
	m_max_ssrc = 7;
	m_max_slrc = 7;
}
void 
MacParameters::initialize_80211a (Phy80211 const*phy)
{
	m_sifs_us = 16;
	m_slot_us = 9;
	/* see section 9.2.10 ieee 802.11-1999 */
	m_pifs_us = m_sifs_us + m_slot_us;
	// 1000m 
	m_max_propagation_delay_us = static_cast<uint64_t> (1000.0 / 300000000.0 * 1000000.0);
	/* Cts_Timeout and Ack_Timeout are specified in the Annex C 
	   (Formal description of MAC operation, see details on the 
	   Trsp timer setting at page 346)
	*/
	ChunkMac80211Hdr hdr;
	hdr.set_type (MAC_80211_CTL_CTS);
	m_cts_timeout_us = m_sifs_us;
	m_cts_timeout_us += phy->calculate_tx_duration_us (hdr.get_size (), 0);
	m_cts_timeout_us += m_max_propagation_delay_us * 2;
	m_cts_timeout_us += m_slot_us;

	hdr.set_type (MAC_80211_CTL_ACK);
	m_ack_timeout_us = m_sifs_us;
	m_ack_timeout_us += phy->calculate_tx_duration_us (hdr.get_size (), 0);
	m_ack_timeout_us += m_max_propagation_delay_us * 2;
	m_ack_timeout_us += m_slot_us;

	m_rx_tx_turnaround = 1; // must be < 2
}

void 
MacParameters::set_slot_time_us (uint64_t slot_time)
{
	m_slot_us = slot_time;
}

void 
MacParameters::set_max_ssrc (uint32_t ssrc)
{
	m_max_ssrc = ssrc;
}
void 
MacParameters::set_max_slrc (uint32_t slrc)
{
	m_max_slrc = slrc;
}
void 
MacParameters::set_rts_cts_threshold (uint32_t threshold)
{
	m_rts_cts_threshold = threshold;
}
void 
MacParameters::set_fragmentation_threshold (uint32_t threshold)
{
	m_fragmentation_threshold = threshold;
}

uint64_t 
MacParameters::get_pifs_us (void) const
{
	return m_pifs_us;
}
uint64_t 
MacParameters::get_sifs_us (void) const
{
	return m_sifs_us;
}
uint64_t 
MacParameters::get_slot_time_us (void) const
{
	return m_slot_us;
}
uint64_t 
MacParameters::get_rx_tx_turnaround (void) const
{
	return m_rx_tx_turnaround;
}
uint64_t 
MacParameters::get_cts_timeout_us (void) const
{
	return m_cts_timeout_us;
}
uint64_t 
MacParameters::get_ack_timeout_us (void) const
{
	return m_ack_timeout_us;
}

uint64_t 
MacParameters::get_beacon_interval_us (void) const
{
	return 1000000; // 1s
}
uint32_t 
MacParameters::get_max_ssrc (void) const
{
	return m_max_ssrc;
}
uint32_t 
MacParameters::get_max_slrc (void) const
{
	return m_max_slrc;
}
uint32_t 
MacParameters::get_rts_cts_threshold (void) const
{
	return m_rts_cts_threshold;
}
uint32_t 
MacParameters::get_fragmentation_threshold (void) const
{
	assert (get_max_msdu_size () / 16 < m_fragmentation_threshold);
	return m_fragmentation_threshold;
}
uint64_t 
MacParameters::get_msdu_lifetime_us (void) const
{
	// 10s
	return 10000000;
}
uint32_t 
MacParameters::get_max_queue_size (void) const
{
	return 400;
}
uint64_t 
MacParameters::get_max_propagation_delay_us (void) const
{
	return m_max_propagation_delay_us;
}

uint32_t 
MacParameters::get_max_msdu_size (void) const
{
	return 2304;
}
double 
MacParameters::get_cap_limit (void) const
{
	return 0.4;
}
double 
MacParameters::get_min_edca_traffic_proportion (void) const
{
	return 0.4;
}

}; // namespace yans
