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

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

namespace yans {

class Buffer {
public:
	Buffer ();
	~Buffer ();

	uint32_t get_size (void) const;
	uint8_t *peek_data (void);

	/* after a call to any of these methods, 
	 * the current position is invalidated.
	 */
	void add_at_start (uint32_t start);
	void add_at_end (uint32_t end);
	void remove_at_start (uint32_t start);
	void remove_at_end (uint32_t end);


	uint32_t get_current (void) const;
	void seek (uint32_t offset);
	void skip (int32_t delta);

	void write_u8  (uint8_t  data);
	void write_u16 (uint16_t data);
	void write_u32 (uint32_t data);
	void write_u64 (uint64_t data);
	void write_hton_u16 (uint16_t data);
	void write_hton_u32 (uint32_t data);
	void write_hton_u64 (uint64_t data);
	void write (uint8_t const*buffer, uint16_t size);

	uint8_t  read_u8 (void);
	uint16_t read_u16 (void);
	uint32_t read_u32 (void);
	uint64_t read_u64 (void);
	uint16_t read_ntoh_u16 (void);
	uint32_t read_ntoh_u32 (void);
	uint64_t read_ntoh_u64 (void);
	void read (uint8_t *buffer, uint16_t size);

private:
	uint8_t *alloc_and_zero (uint32_t size) const;
	uint8_t *get_start (void) const;

	static uint32_t get_prefered_start (void);
	static uint32_t get_prefered_size (void);
	static void record_buffer_stats (uint32_t start, uint32_t end);
	static uint32_t m_total_start;
	static uint32_t m_total_end;

	uint32_t m_size;
	uint32_t m_current;
	uint32_t m_start;
	uint8_t *m_data;
	uint32_t m_real_size;
	uint32_t m_total_added_start;
	uint32_t m_total_added_end;
};

}; // namespace yans
#endif /* BUFFER_H */
