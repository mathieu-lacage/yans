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

#ifndef CHUNK_IPV4_H
#define CHUNK_IPV4_H

#include "chunk.h"
#include "ipv4-address.h"

class ChunkIpv4 : public Chunk {
public:
	ChunkIpv4 ();
	virtual ~ChunkIpv4 ();

	void set_payload_size (uint16_t size);
	void set_identification (uint16_t identification);
	void set_tos (uint8_t);
	void set_more_fragments (void);
	void set_last_fragment (void);
	void set_dont_fragment (void);
	void set_may_fragment (void);
	void set_fragment_offset (uint16_t offset);
	void set_ttl (uint8_t);
	void set_protocol (uint8_t);
	void set_source (Ipv4Address source);
	void set_destination (Ipv4Address destination);


	uint16_t get_payload_size (void) const;
	uint16_t get_identification (void) const;
	uint8_t get_tos (void) const;
	bool is_last_fragment (void) const;
	bool is_dont_fragment (void) const;
	uint16_t get_fragment_offset (void) const;
	uint8_t get_ttl (void) const;
	uint8_t get_protocol (void) const;
	Ipv4Address get_source (void) const;
	Ipv4Address get_destination (void) const;

	bool is_checksum_ok (void) const;

	virtual uint32_t get_size (void) const;
	virtual Chunk *copy (void) const;
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os) const;
private:

	enum Flags_e {
		DONT_FRAGMENT = (1<<0),
		MORE_FRAGMENTS = (1<<1)
	};

	uint16_t m_payload_size;
	uint16_t m_identification;
	uint32_t m_tos : 8;
	uint32_t m_ttl : 8;
	uint32_t m_protocol : 8;
	uint32_t m_flags : 3;
	uint16_t m_fragment_offset : 13;
	Ipv4Address m_source;
	Ipv4Address m_destination;
};


#endif /* CHUNK_IPV4_H */
