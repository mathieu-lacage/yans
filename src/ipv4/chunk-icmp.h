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

#ifndef CHUNK_ICMP_H
#define CHUNK_ICMP_H

#include "chunk.h"
#include "ipv4-address.h"

class ChunkIcmp: public Chunk {
public:
	ChunkIcmp ();
	virtual ~ChunkIcmp ();

	uint8_t get_type (void) const;
	uint8_t get_code (void) const;
	void set_code (uint8_t code);

	void set_time_exceeded (void);

	virtual uint32_t get_size (void) const;
	virtual Chunk *copy (void) const;
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os) const;

	enum {
		ECHO_REPLY    = 0,
		UNREACH       = 3,
		SOURCE_QUENCH = 4,
		REDIRECT      = 5,
		ECHO          = 8,
		TIME_EXCEEDED = 11,
		PARAMETER_PROBLEM   = 12,
		TIMESTAMP           = 13,
		TIMESTAMP_REPLY     = 14,
		INFORMATION_REQUEST = 15,
		INFORMATION_REPLY   = 16
	};

private:
	/* unimplemented for now. */
	void set_identifier_and_sequence (uint16_t identifier, uint16_t sequence_number);
	void set_redirect (uint8_t code, Ipv4Address gateway);
	void set_echo (uint8_t echo_data_size);
	void set_echo_reply (uint8_t echo_data_size);

	uint8_t m_type;
	uint8_t m_code;
	uint16_t m_checksum;
	uint16_t m_identifier;
	uint16_t m_seq_number;
	Ipv4Address m_gateway;
	uint8_t m_echo_data_size;
	uint32_t m_org_timestamp;
	uint32_t m_rx_timestamp;
	uint32_t m_tx_timestamp;

};


#endif /* CHUNK_ICMP_H */
