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
#ifndef CHUNK_MAC_80211_HDR_H
#define CHUNK_MAC_80211_HDR_H

#include "chunk.h"
#include "mac-address.h"
#include <stdint.h>

namespace yans {

enum Mac80211Type_e {
	MAC_80211_CTL_RTS = 0,
	MAC_80211_CTL_CTS,
	MAC_80211_CTL_ACK,
	MAC_80211_CTL_BACKREQ,
	MAC_80211_CTL_BACKRESP,

	MAC_80211_MGT_BEACON,
	MAC_80211_MGT_ASSOCIATION_REQUEST,
	MAC_80211_MGT_ASSOCIATION_RESPONSE,
	MAC_80211_MGT_DISASSOCIATION,
	MAC_80211_MGT_REASSOCIATION_REQUEST,
	MAC_80211_MGT_REASSOCIATION_RESPONSE,
	MAC_80211_MGT_PROBE_REQUEST,
	MAC_80211_MGT_PROBE_RESPONSE,
	MAC_80211_MGT_AUTHENTICATION,
	MAC_80211_MGT_DEAUTHENTICATION,

	MAC_80211_DATA,
	MAC_80211_DATA_CFACK,
	MAC_80211_DATA_CFPOLL,
	MAC_80211_DATA_CFACK_CFPOLL,
	MAC_80211_DATA_NULL,
	MAC_80211_DATA_NULL_CFACK,
	MAC_80211_DATA_NULL_CFPOLL,
	MAC_80211_DATA_NULL_CFACK_CFPOLL,
	MAC_80211_QOSDATA,
	MAC_80211_QOSDATA_CFACK,
	MAC_80211_QOSDATA_CFPOLL,
	MAC_80211_QOSDATA_CFACK_CFPOLL,
	MAC_80211_QOSDATA_NULL,
	MAC_80211_QOSDATA_NULL_CFPOLL,
	MAC_80211_QOSDATA_NULL_CFACK_CFPOLL,
};

class ChunkMac80211Hdr : public Chunk {
public:
	ChunkMac80211Hdr ();

	void set_ds_from (void);
	void set_ds_not_from (void);
	void set_ds_to (void);
	void set_ds_not_to (void);
	void set_addr1 (MacAddress address);
	void set_addr2 (MacAddress address);
	void set_addr3 (MacAddress address);
	void set_addr4 (MacAddress address);
	void set_type (enum Mac80211Type_e type);
	void set_duration (uint16_t duration);
	void set_duration_us (uint64_t duration);
	void set_id (uint16_t id);
	void set_sequence_number (uint16_t seq);
	void set_fragment_number (uint8_t frag);
	void set_no_more_fragments (void);
	void set_more_fragments (void);
	void set_retry (void);
	void set_no_retry (void);
	void set_qos_tid (uint8_t tid);
	void set_qos_txop_limit (uint8_t txop);

	MacAddress get_addr1 (void) const;
	MacAddress get_addr2 (void) const;
	MacAddress get_addr3 (void) const;
	MacAddress get_addr4 (void) const;
	enum Mac80211Type_e get_type (void) const;
	bool is_data (void) const;
	bool is_qos_data (void) const;
	bool is_ctl (void) const;
	bool is_mgt (void) const;
	bool is_cfpoll (void) const;
	bool is_rts (void) const;
	bool is_cts (void) const;
	bool is_ack (void) const;
	uint16_t get_duration (void) const;
	uint64_t get_duration_us (void) const;
	uint16_t get_sequence_control (void) const;
	bool is_retry (void) const;
	bool is_more_fragments (void) const;
	bool is_qos_block_ack (void) const;
	bool is_qos_no_ack (void) const;
	bool is_qos_ack (void) const;
	uint8_t get_qos_tid (void) const;
	uint8_t get_qos_txop_limit (void) const;

	void set_duration_s (double duration);
	uint32_t get_size (void) const;
	char const *get_type_string (void) const;


	virtual ~ChunkMac80211Hdr ();

	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	uint16_t get_frame_control (void) const;
	uint16_t get_qos_control (void) const;
	void set_frame_control (uint16_t control);
	void set_sequence_control (uint16_t seq);
	void set_qos_control (uint16_t qos);

	uint16_t m_ctrl_type      : 2;
	uint16_t m_ctrl_subtype   : 4;
	uint16_t m_ctrl_to_ds     : 1;
	uint16_t m_ctrl_from_ds   : 1;
	uint16_t m_ctrl_more_frag : 1;
	uint16_t m_ctrl_retry     : 1;
	uint16_t m_ctrl_pwr_mgt   : 1;
	uint16_t m_ctrl_more_data : 1;
	uint16_t m_ctrl_wep       : 1;
	uint16_t m_ctrl_order     : 1;
	uint16_t m_duration;
	MacAddress m_addr1;
	MacAddress m_addr2;
	MacAddress m_addr3;
	uint16_t m_seq_frag : 4;
	uint16_t m_seq_seq  : 12;
	MacAddress m_addr4;
	uint16_t m_qos_tid : 4;
	uint16_t m_qos_eosp : 1;
	uint16_t m_qos_ack_policy : 2;
	uint16_t m_qos_stuff: 8;
};

}; // namespace yans



#endif /* CHUNK_MAC_80211_HDR_H */
