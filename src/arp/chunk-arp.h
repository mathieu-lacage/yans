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

#ifndef CHUNK_ARP_H
#define CHUNK_ARP_H

#include "chunk.h"
#include "mac-address.h"
#include "ipv4-address.h"

namespace yans {

class ChunkArp : public Chunk {
 public:
	virtual ~ChunkArp ();

	void set_request (MacAddress source_hardware_address,
			  Ipv4Address source_protocol_address,
			  Ipv4Address destination_protocol_address);
	void set_reply (MacAddress source_hardware_address,
			Ipv4Address source_protocol_address,
			MacAddress destination_hardware_address,
			Ipv4Address destination_protocol_address);
	bool is_request (void) const;
	bool is_reply (void) const;
	MacAddress get_source_hardware_address (void);
	MacAddress get_destination_hardware_address (void);
	Ipv4Address get_source_ipv4_address (void);
	Ipv4Address get_destination_ipv4_address (void);

	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;

private:
	uint32_t get_size (void) const;
	enum ArpType_e {
		ARP_TYPE_REQUEST = 1,
		ARP_TYPE_REPLY   = 2
	};
	uint16_t m_type;
	MacAddress m_mac_source;
	MacAddress m_mac_dest;
	Ipv4Address m_ipv4_source;
	Ipv4Address m_ipv4_dest;
};

}; // namespace yans

#endif /* CHUNK_ARP_H */
