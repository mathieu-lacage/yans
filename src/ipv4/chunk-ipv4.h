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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#ifndef CHUNK_IPV4_H
#define CHUNK_IPV4_H

#include "chunk.h"
#include "ipv4-address.h"

class ChunkIpv4 : public Chunk {
public:
	ChunkIpv4 ();
	virtual ~ChunkIpv4 ();

	void set_payload_size (uint16_t size);
	uint16_t get_payload_size (void);

	void set_tos (uint8_t);
	uint8_t get_tos (void);
	void set_id (uint16_t);
	uint16_t get_id (void);

	void set_more_fragments (void);
	void set_last_fragment (void);
	bool is_last_fragment (void);

	void set_fragment_offset (uint16_t offset);
	uint16_t get_fragment_offset (void);

	void set_ttl (uint8_t);
	uint8_t get_ttl (void);
	
	uint8_t get_protocol (void);
	void set_protocol (uint8_t);

	void set_source (Ipv4Address source);
	Ipv4Address get_source (void);

	void set_destination (Ipv4Address destination);
	Ipv4Address get_destination (void);

	bool is_checksum_ok (void);

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	uint8_t m_ver_ihl;
	uint8_t m_tos;
	uint16_t m_id;
	uint16_t m_total_length;
	uint16_t m_fragment_offset;
	uint8_t m_ttl;
	uint8_t m_protocol;
	uint16_t m_checksum;
	uint32_t m_source;
	uint32_t m_destination;
	uint16_t m_payload_size;
};


#endif /* CHUNK_IPV4_H */
