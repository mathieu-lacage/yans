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

#ifndef CHUNK_TCP_H
#define CHUNK_TCP_H

#include "chunk.h"
#include "ipv4-address.h"

namespace yans {

class ChunkTcp : public Chunk {
public:
	ChunkTcp ();
	virtual ~ChunkTcp ();

	static void enable_checksums (void);

	void set_source_port (uint16_t port);
	void set_destination_port (uint16_t port);
	void set_sequence_number (uint32_t sequence);
	void set_window_size (uint16_t size);
	void set_ack_number (uint32_t ack);

	uint16_t get_source_port (void);
	uint16_t get_destination_port (void);
	uint32_t get_sequence_number (void);
	uint32_t get_ack_number (void);
	uint16_t get_window_size (void);
	uint16_t get_urgent_pointer (void);

	uint16_t get_option_mss (void) const;
	uint32_t get_option_timestamp_value (void) const;
	uint32_t get_option_timestamp_reply (void) const;
	uint8_t  get_option_windowscale (void) const;
	void enable_option_mss (uint16_t mss);
	void enable_option_timestamp (uint32_t value, uint32_t reply);
	void enable_option_windowscale (uint8_t log_scale);
	bool is_option_mss (void) const;
	bool is_option_timestamp (void) const;
	bool is_option_windowscale (void) const;


	void enable_flag_syn (void);
	void enable_flag_fin (void);
	void enable_flag_rst (void);
	void enable_flag_ack (void);
	void enable_flag_urg (uint16_t ptr);
	void enable_flag_psh (void);

	void disable_flags (void);
	void disable_flag_syn (void);
	void disable_flag_fin (void);
	void disable_flag_rst (void);
	void disable_flag_ack (void);
	void disable_flag_urg (void);
	void disable_flag_psh (void);

	bool is_flag_syn (void) const;
	bool is_flag_fin (void) const;
	bool is_flag_rst (void) const;
	bool is_flag_ack (void) const;
	bool is_flag_urg (void) const;
	bool is_flag_psh (void) const;

	bool is_checksum_ok (void);

	void initialize_checksum (Ipv4Address source,
				  Ipv4Address destination,
				  uint8_t protocol,
				  uint16_t payload_size);

	uint32_t get_size (void) const;

private:
	virtual void print (std::ostream *os) const;
	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const *buffer);
	virtual void remove_from (Buffer *buffer);
	enum {
		URG = 5,
		ACK = 4,
		PSH = 3,
		RST = 2,
		SYN = 1,
		FIN = 0
	};
	bool is_flag (uint8_t n) const;
	void enable_flag (uint8_t n);
	void disable_flag (uint8_t n);
	uint32_t get_raw_size (void) const;
	uint32_t get_padding (void) const;

	uint16_t m_source_port;
	uint16_t m_destination_port;
	uint32_t m_sequence_number;
	uint32_t m_ack_number;
	uint8_t m_header_length;
	uint8_t m_flags;
	uint16_t m_window_size;
	uint16_t m_urgent_pointer;

	uint32_t m_has_option_mss         : 1;
	uint32_t m_has_option_timestamp   : 1;
	uint32_t m_has_option_windowscale : 1;
	uint32_t m_is_checksum_ok         : 1;
	
	uint16_t m_option_mss;
	uint8_t m_option_windowscale;
	uint32_t m_option_timestamp_value;
	uint32_t m_option_timestamp_reply;
	uint16_t m_initial_checksum;

	static bool m_calc_checksum;
};

}; // namespace yans

#endif /* CHUNK_TCP_H */
