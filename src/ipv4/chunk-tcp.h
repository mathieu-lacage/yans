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

class ChunkTcp : public Chunk {
public:
	ChunkTcp ();
	virtual ~ChunkTcp ();

	void set_source_port (uint16_t port);
	void set_destination_port (uint16_t port);
	void set_sequence_number (uint32_t sequence);
	void set_window_size (uint16_t size);

	uint16_t get_source_port (void);
	uint16_t get_destination_port (void);
	uint32_t get_sequence_number (void);
	uint32_t get_ack_number (void);
	uint16_t get_window_size (void);
	uint16_t get_urgent_pointer (void);
	uint16_t get_option_mss (void);

	void enable_flag_syn (void);
	void enable_flag_fin (void);
	void enable_flag_rst (void);
	void enable_flag_ack (uint32_t ack);
	void enable_flag_urg (uint16_t ptr);
	void enable_flag_psh (void);
	void enable_option_mss (uint16_t mss);

	void disable_flag_syn (void);
	void disable_flag_fin (void);
	void disable_flag_rst (void);
	void disable_flag_ack (void);
	void disable_flag_urg (void);
	void disable_flag_psh (void);

	bool is_flag_syn (void);
	bool is_flag_fin (void);
	bool is_flag_rst (void);
	bool is_flag_ack (void);
	bool is_flag_urg (void);
	bool is_flag_psh (void);
	bool is_option_mss (void);

	bool is_checksum_ok (void);

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	enum {
		URG = 2,
		ACK = 3,
		PSH = 4,
		RST = 5,
		SYN = 6,
		FIN = 7
	};
	bool is_flag (uint8_t n);
	void enable_flag (uint8_t n);
	void disable_flag (uint8_t n);
	uint16_t m_source_port;
	uint16_t m_destination_port;
	uint32_t m_sequence_number;
	uint32_t m_ack_number;
	uint8_t m_header_length;
	uint8_t m_flags;
	uint16_t m_window_size;
	uint16_t m_checksum;
	uint16_t m_urgent_pointer;
	uint32_t m_mss;
};

#endif /* CHUNK_TCP_H */
