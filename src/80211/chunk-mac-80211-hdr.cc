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
#include "chunk-mac-80211-hdr.h"
#include "buffer.h"
#include <cassert>

namespace yans {

enum {
	TYPE_MGT = 0,
	TYPE_CTL  = 1,
	TYPE_DATA = 2
};

enum {
	SUBTYPE_CTL_BACKREQ = 8,
	SUBTYPE_CTL_BACKRESP = 9,
	SUBTYPE_CTL_RTS = 11,
	SUBTYPE_CTL_CTS = 12,
	SUBTYPE_CTL_ACK = 13
};


ChunkMac80211Hdr::ChunkMac80211Hdr ()
{}
ChunkMac80211Hdr::~ChunkMac80211Hdr ()
{}

void 
ChunkMac80211Hdr::set_addr1 (MacAddress address)
{
	m_addr1 = address;
}
void 
ChunkMac80211Hdr::set_addr2 (MacAddress address)
{
	m_addr2 = address;
}
void 
ChunkMac80211Hdr::set_addr3 (MacAddress address)
{
	m_addr3 = address;
}
void 
ChunkMac80211Hdr::set_addr4 (MacAddress address)
{
	m_addr4 = address;
}
void 
ChunkMac80211Hdr::set_type (enum Mac80211Type_e type)
{
	switch (type) {
	case MAC_80211_CTL_BACKREQ:
		m_ctrl_type = TYPE_CTL;
		m_ctrl_subtype = SUBTYPE_CTL_BACKREQ;
		break;
	case MAC_80211_CTL_BACKRESP:
		m_ctrl_type = TYPE_CTL;
		m_ctrl_subtype = SUBTYPE_CTL_BACKRESP;
		break;
	case MAC_80211_CTL_RTS:
		m_ctrl_type = TYPE_CTL;
		m_ctrl_subtype = SUBTYPE_CTL_RTS;
		break;
	case MAC_80211_CTL_CTS:
		m_ctrl_type = TYPE_CTL;
		m_ctrl_subtype = SUBTYPE_CTL_CTS;
		break;
	case MAC_80211_CTL_ACK:
		m_ctrl_type = TYPE_CTL;
		m_ctrl_subtype = SUBTYPE_CTL_ACK;
		break;
	case MAC_80211_MGT_ASSOCIATION_REQUEST:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 0;
		break;
	case MAC_80211_MGT_ASSOCIATION_RESPONSE:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 1;
		break;
	case MAC_80211_MGT_REASSOCIATION_REQUEST:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 2;
		break;
	case MAC_80211_MGT_REASSOCIATION_RESPONSE:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 3;
		break;
	case MAC_80211_MGT_PROBE_REQUEST:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 4;
		break;
	case MAC_80211_MGT_PROBE_RESPONSE:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 5;
		break;
	case MAC_80211_MGT_BEACON:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 8;
		break;
	case MAC_80211_MGT_DISASSOCIATION:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 10;
		break;
	case MAC_80211_MGT_AUTHENTICATION:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 11;
		break;
	case MAC_80211_MGT_DEAUTHENTICATION:
		m_ctrl_type = TYPE_MGT;
		m_ctrl_subtype = 12;
		break;

	case MAC_80211_DATA:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 0;
		break;
	case MAC_80211_DATA_CFACK:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 1;
		break;
	case MAC_80211_DATA_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 2;
		break;
	case MAC_80211_DATA_CFACK_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 3;
		break;
	case MAC_80211_DATA_NULL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 4;
		break;
	case MAC_80211_DATA_NULL_CFACK:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 5;
		break;
	case MAC_80211_DATA_NULL_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 6;
		break;
	case MAC_80211_DATA_NULL_CFACK_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 7;
		break;
	case MAC_80211_QOSDATA:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 8;
		break;
	case MAC_80211_QOSDATA_CFACK:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 9;
		break;
	case MAC_80211_QOSDATA_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 10;
		break;
	case MAC_80211_QOSDATA_CFACK_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 11;
		break;
	case MAC_80211_QOSDATA_NULL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 12;
		break;
	case MAC_80211_QOSDATA_NULL_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 14;
		break;
	case MAC_80211_QOSDATA_NULL_CFACK_CFPOLL:
		m_ctrl_type = TYPE_DATA;
		m_ctrl_subtype = 15;
		break;
	}
	m_ctrl_to_ds = 0;
	m_ctrl_from_ds = 0;
}
void 
ChunkMac80211Hdr::set_duration (uint16_t duration)
{
	m_duration = duration;
}
void 
ChunkMac80211Hdr::set_duration_us (uint64_t duration_us)
{
	assert (duration_us <= (1<<16) - 1);
	m_duration = static_cast<uint16_t> (duration_us);
}

void ChunkMac80211Hdr::set_id (uint16_t id)
{
	m_duration = id;
}
void ChunkMac80211Hdr::set_sequence_number (uint16_t seq)
{
	m_seq_seq = seq;
}
void ChunkMac80211Hdr::set_fragment_number (uint8_t frag)
{
	m_seq_frag = frag;
}
void ChunkMac80211Hdr::set_no_more_fragments (void)
{
	m_ctrl_more_frag = 0;
}
void ChunkMac80211Hdr::set_more_fragments (void)
{
	m_ctrl_more_frag = 1;
}
void ChunkMac80211Hdr::set_retry (void)
{
	m_ctrl_retry = 1;
}
void ChunkMac80211Hdr::set_no_retry (void)
{
	m_ctrl_retry = 0;
}
void ChunkMac80211Hdr::set_qos_tid (uint8_t tid)
{
	m_qos_tid = tid;
}
void ChunkMac80211Hdr::set_qos_txop_limit (uint8_t txop)
{
	m_qos_stuff = txop;
}

MacAddress 
ChunkMac80211Hdr::get_addr1 (void) const
{
	return m_addr1;
}
MacAddress 
ChunkMac80211Hdr::get_addr2 (void) const
{
	return m_addr2;
}
MacAddress 
ChunkMac80211Hdr::get_addr3 (void) const
{
	return m_addr3;
}
MacAddress 
ChunkMac80211Hdr::get_addr4 (void) const
{
	return m_addr4;
}
enum Mac80211Type_e 
ChunkMac80211Hdr::get_type (void) const
{
	switch (m_ctrl_type) {
	case TYPE_MGT:
		switch (m_ctrl_subtype) {
		case 0:
			return MAC_80211_MGT_ASSOCIATION_REQUEST;
			break;
		case 1:
			return MAC_80211_MGT_ASSOCIATION_RESPONSE;
			break;
		case 2:
			return MAC_80211_MGT_REASSOCIATION_REQUEST;
			break;
		case 3:
			return MAC_80211_MGT_REASSOCIATION_RESPONSE;
			break;
		case 4:
			return MAC_80211_MGT_PROBE_REQUEST;
			break;
		case 5:
			return MAC_80211_MGT_PROBE_RESPONSE;
			break;
		case 8:
			return MAC_80211_MGT_BEACON;
			break;
		case 10:
			return MAC_80211_MGT_DISASSOCIATION;
			break;
		case 11:
			return MAC_80211_MGT_AUTHENTICATION;
			break;
		case 12:
			return MAC_80211_MGT_DEAUTHENTICATION;
			break;

		}
		break;
	case TYPE_CTL:
		switch (m_ctrl_subtype) {
		case SUBTYPE_CTL_BACKREQ:
			return MAC_80211_CTL_BACKREQ;
			break;
		case SUBTYPE_CTL_BACKRESP:
			return MAC_80211_CTL_BACKRESP;
			break;
		case SUBTYPE_CTL_RTS:
			return MAC_80211_CTL_RTS;
			break;
		case SUBTYPE_CTL_CTS:
			return MAC_80211_CTL_CTS;
			break;
		case SUBTYPE_CTL_ACK:
			return MAC_80211_CTL_ACK;
			break;
		}
		break;
	case TYPE_DATA:
		switch (m_ctrl_subtype) {
		case 0:
			return MAC_80211_DATA;
			break;
		case 1:
			return MAC_80211_DATA_CFACK;
			break;
		case 2:
			return MAC_80211_DATA_CFPOLL;
			break;
		case 3:
			return MAC_80211_DATA_CFACK_CFPOLL;
			break;
		case 4:
			return MAC_80211_DATA_NULL;
			break;
		case 5:
			return MAC_80211_DATA_NULL_CFACK;
			break;
		case 6:
			return MAC_80211_DATA_NULL_CFPOLL;
			break;
		case 7:
			return MAC_80211_DATA_NULL_CFACK_CFPOLL;
			break;
		case 8:
			return MAC_80211_QOSDATA;
			break;
		case 9:
			return MAC_80211_QOSDATA_CFACK;
			break;
		case 10:
			return MAC_80211_QOSDATA_CFPOLL;
			break;
		case 11:
			return MAC_80211_QOSDATA_CFACK_CFPOLL;
			break;
		case 12:
			return MAC_80211_QOSDATA_NULL;
			break;
		case 14:
			return MAC_80211_QOSDATA_NULL_CFPOLL;
			break;
		case 15:
			return MAC_80211_QOSDATA_NULL_CFACK_CFPOLL;
			break;

		}
		break;
	}
	// NOTREACHED
	assert (false);
	return (enum Mac80211Type_e)-1;
}
bool 
ChunkMac80211Hdr::is_data (void) const
{
	return (m_ctrl_type == TYPE_DATA)?true:false;
		
}
bool 
ChunkMac80211Hdr::is_qos_data (void) const
{
	return (m_ctrl_type == TYPE_DATA && (m_ctrl_subtype & 0x08))?true:false;
}
bool 
ChunkMac80211Hdr::is_ctl (void) const
{
	return (m_ctrl_type == TYPE_CTL)?true:false;
}
bool 
ChunkMac80211Hdr::is_mgt (void) const
{
	return (m_ctrl_type == TYPE_MGT)?true:false;
}
bool 
ChunkMac80211Hdr::is_cfpoll (void) const
{
	switch (get_type ()) {
	case MAC_80211_DATA_CFPOLL:
	case MAC_80211_DATA_CFACK_CFPOLL:
	case MAC_80211_DATA_NULL_CFPOLL:
	case MAC_80211_DATA_NULL_CFACK_CFPOLL:
	case MAC_80211_QOSDATA_CFPOLL:
	case MAC_80211_QOSDATA_CFACK_CFPOLL:
	case MAC_80211_QOSDATA_NULL_CFPOLL:
	case MAC_80211_QOSDATA_NULL_CFACK_CFPOLL:
		return true;
		break;
	default:
		return false;
		break;
	}
}
bool 
ChunkMac80211Hdr::is_rts (void) const
{
	return (get_type () == MAC_80211_CTL_RTS)?true:false;
}
bool 
ChunkMac80211Hdr::is_cts (void) const
{
	return (get_type () == MAC_80211_CTL_CTS)?true:false;
}
bool 
ChunkMac80211Hdr::is_ack (void) const
{
	return (get_type () == MAC_80211_CTL_ACK)?true:false;
}

uint16_t 
ChunkMac80211Hdr::get_duration (void) const
{
	return m_duration;
}
uint16_t 
ChunkMac80211Hdr::get_sequence_control (void) const
{
	return (m_seq_frag << 12) | m_seq_seq;
}
bool 
ChunkMac80211Hdr::is_retry (void) const
{
	return (m_ctrl_retry == 1)?true:false;
}
bool 
ChunkMac80211Hdr::is_more_fragments (void) const
{
	return (m_ctrl_more_frag == 1)?true:false;
}
bool 
ChunkMac80211Hdr::is_qos_block_ack (void) const
{
	assert (is_qos_data ());
	return (m_qos_ack_policy == 3)?true:false;
}
bool 
ChunkMac80211Hdr::is_qos_no_ack (void) const
{
	assert (is_qos_data ());
	return (m_qos_ack_policy == 1)?true:false;
}
bool 
ChunkMac80211Hdr::is_qos_ack (void) const
{
	assert (is_qos_data ());
	return (m_qos_ack_policy == 0)?true:false;
}
uint8_t
ChunkMac80211Hdr::get_qos_tid (void) const
{
	assert (is_qos_data ());
	return m_qos_tid;
}
uint8_t 
ChunkMac80211Hdr::get_qos_txop_limit (void) const
{
	assert (is_qos_data ());
	return m_qos_stuff;
}

uint16_t
ChunkMac80211Hdr::get_frame_control (void) const
{
	uint16_t val = 0;
	val |= m_ctrl_type << 2;
	val |= m_ctrl_subtype << 4;
	val |= m_ctrl_to_ds << 8;
	val |= m_ctrl_from_ds << 9;
	val |= m_ctrl_more_frag << 10;
	val |= m_ctrl_retry << 11;
	val |= m_ctrl_more_data << 13;
	val |= m_ctrl_wep << 14;
	val |= m_ctrl_order << 15;
	return val;
}

uint16_t
ChunkMac80211Hdr::get_qos_control (void) const
{
	uint16_t val = 0;
	val |= m_qos_tid;
	val |= m_qos_eosp << 4;
	val |= m_qos_ack_policy << 5;
	val |= m_qos_stuff << 8;
	return val;
}

void
ChunkMac80211Hdr::set_frame_control (uint16_t ctrl)
{
	m_ctrl_type = (ctrl >> 2) & 0x03;
	m_ctrl_subtype = (ctrl >> 4) & 0x0f;
	m_ctrl_to_ds = (ctrl >> 8) & 0x01;
	m_ctrl_from_ds = (ctrl >> 9) & 0x01;
	m_ctrl_more_frag = (ctrl >> 10) & 0x01;
	m_ctrl_retry = (ctrl >> 11) & 0x01;
	m_ctrl_more_data = (ctrl >> 13) & 0x01;
	m_ctrl_wep = (ctrl >> 14) & 0x01;
	m_ctrl_order = (ctrl >> 15) & 0x01;
}
void 
ChunkMac80211Hdr::set_sequence_control (uint16_t seq)
{
	m_seq_frag = seq & 0x0f;
	m_seq_seq = (seq >> 4) & 0x0fff;
}
void
ChunkMac80211Hdr::set_qos_control (uint16_t qos)
{
	m_qos_tid = qos & 0x000f;
	m_qos_eosp = (qos >> 4) & 0x0001;
	m_qos_ack_policy = (qos >> 5) & 0x0003;
	m_qos_stuff = (qos >> 8) & 0x00ff;
}


void 
ChunkMac80211Hdr::set_duration_s (double duration)
{
	uint16_t us = (uint16_t)(duration * 1000000);
	us &= 0x7fff;
	set_duration (us);
}

uint32_t 
ChunkMac80211Hdr::get_size (void) const
{
	uint32_t size = 0;
	switch (m_ctrl_type) {
	case TYPE_MGT:
		size = 2+2+6+6+6+2;
		break;
	case TYPE_CTL:
		switch (m_ctrl_subtype) {
		case SUBTYPE_CTL_RTS:
			size = 2+2+6+6;
			break;
		case SUBTYPE_CTL_CTS:
		case SUBTYPE_CTL_ACK:
			size = 2+2+6;
			break;
		case SUBTYPE_CTL_BACKREQ:
		case SUBTYPE_CTL_BACKRESP:
			// NOT IMPLEMENTED
			assert (false);
			break;
		}
		break;
	case TYPE_DATA:
		size = 2+2+6+6+6+2;
		if (m_ctrl_to_ds && m_ctrl_from_ds) {
			size += 6;
		}
		if (m_ctrl_subtype & 0x08) {
			size += 2;
		}
		break;
	}
	return size;
}
char const *
ChunkMac80211Hdr::get_type_string (void)
{
#define FOO(x) \
case MAC_80211_ ## x: \
	return #x; \
	break;

	switch (get_type ()) {
		FOO (CTL_RTS);
		FOO (CTL_CTS);
		FOO (CTL_ACK);
		FOO (CTL_BACKREQ);
		FOO (CTL_BACKRESP);

		FOO (MGT_BEACON);
		FOO (MGT_ASSOCIATION_REQUEST);
		FOO (MGT_ASSOCIATION_RESPONSE);
		FOO (MGT_DISASSOCIATION);
		FOO (MGT_REASSOCIATION_REQUEST);
		FOO (MGT_REASSOCIATION_RESPONSE);
		FOO (MGT_PROBE_REQUEST);
		FOO (MGT_PROBE_RESPONSE);
		FOO (MGT_AUTHENTICATION);
		FOO (MGT_DEAUTHENTICATION);
		
		FOO (DATA);
		FOO (DATA_CFACK);
		FOO (DATA_CFPOLL);
		FOO (DATA_CFACK_CFPOLL);
		FOO (DATA_NULL);
		FOO (DATA_NULL_CFACK);
		FOO (DATA_NULL_CFPOLL);
		FOO (DATA_NULL_CFACK_CFPOLL);
		FOO (QOSDATA);
		FOO (QOSDATA_CFACK);
		FOO (QOSDATA_CFPOLL);
		FOO (QOSDATA_CFACK_CFPOLL);
		FOO (QOSDATA_NULL);
		FOO (QOSDATA_NULL_CFPOLL);
		FOO (QOSDATA_NULL_CFACK_CFPOLL);
	default:
		return "ERROR";
	}
#undef FOO
}


void 
ChunkMac80211Hdr::add_to (Buffer *buffer) const
{
	switch (m_ctrl_type) {
	case TYPE_MGT:
		buffer->add_at_start (2+2+6+6+6+2);
		buffer->seek (0);
		buffer->write_hton_u16 (get_frame_control ());
		buffer->write_hton_u16 (m_duration);
		m_addr1.serialize (buffer);
		m_addr2.serialize (buffer);
		m_addr3.serialize (buffer);
		buffer->write_hton_u16 (get_sequence_control ());
		break;
	case TYPE_CTL:
		switch (m_ctrl_subtype) {
		case SUBTYPE_CTL_RTS:
			buffer->add_at_start (2+2+6+6);
			buffer->seek (0);
			buffer->write_hton_u16 (get_frame_control ());
			buffer->write_hton_u16 (m_duration);
			m_addr1.serialize (buffer);
			m_addr2.serialize (buffer);
			break;
		case SUBTYPE_CTL_CTS:
		case SUBTYPE_CTL_ACK:
			buffer->add_at_start (2+2+6);
			buffer->seek (0);
			buffer->write_hton_u16 (get_frame_control ());
			buffer->write_hton_u16 (m_duration);
			m_addr1.serialize (buffer);
			break;
		case SUBTYPE_CTL_BACKREQ:
		case SUBTYPE_CTL_BACKRESP:
			// NOT IMPLEMENTED
			assert (false);
			break;
		}
		break;
	case TYPE_DATA: {
		uint32_t size = 2+2+6+6+6+2;
		if (m_ctrl_to_ds && m_ctrl_from_ds) {
			size += 6;
		}
		if (m_ctrl_subtype & 0x08) {
			size += 2;
		}
		buffer->add_at_start (size);
		buffer->seek (0);
		buffer->write_hton_u16 (get_frame_control ());
		buffer->write_hton_u16 (m_duration);
		m_addr1.serialize (buffer);
		m_addr2.serialize (buffer);
		m_addr3.serialize (buffer);
		buffer->write_hton_u16 (get_sequence_control ());
		if (m_ctrl_to_ds && m_ctrl_from_ds) {
			m_addr4.serialize (buffer);
		}
		if (m_ctrl_subtype & 0x08) {
			buffer->write_hton_u16 (get_qos_control ());
		}
		} break;
	}
}
void 
ChunkMac80211Hdr::remove_from (Buffer *buffer)
{
	buffer->seek (0);
	uint32_t frame_control = buffer->read_ntoh_u16 ();
	set_frame_control (frame_control);
	m_duration = buffer->read_ntoh_u16 ();
	m_addr1.deserialize (buffer);
	switch (m_ctrl_type) {
	case TYPE_MGT:
		m_addr2.deserialize (buffer);
		m_addr3.deserialize (buffer);
		set_sequence_control (buffer->read_ntoh_u16 ());
		break;
	case TYPE_CTL:
		switch (m_ctrl_subtype) {
		case SUBTYPE_CTL_RTS:
			m_addr2.deserialize (buffer);
			break;
		case SUBTYPE_CTL_CTS:
		case SUBTYPE_CTL_ACK:
			break;
		case SUBTYPE_CTL_BACKREQ:
		case SUBTYPE_CTL_BACKRESP:
			// NOT IMPLEMENTED
			assert (false);
			break;
		}
		break;
	case TYPE_DATA:
		uint32_t size = 2+2+6+6+6+2;
		if (m_ctrl_to_ds && m_ctrl_from_ds) {
			size += 6;
		}
		if (m_ctrl_subtype & 0x08) {
			size += 2;
		}
		m_addr2.deserialize (buffer);
		m_addr3.deserialize (buffer);
		set_sequence_control (buffer->read_ntoh_u16 ());
		if (m_ctrl_to_ds && m_ctrl_from_ds) {
			m_addr4.deserialize (buffer);
		}
		if (m_ctrl_subtype & 0x08) {
			set_qos_control (buffer->read_ntoh_u16 ());
		}
		break;
	}
	buffer->remove_at_start (buffer->get_current ());
}
void 
ChunkMac80211Hdr::print (std::ostream *os) const
{}

}; // namespace yans
