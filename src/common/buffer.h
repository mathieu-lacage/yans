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
	class Iterator {
	public:
		Iterator ();
		Iterator (Iterator const&o);
		Iterator &operator = (Iterator const &o);
		void next (void);
		void prev (void);
		void next (uint32_t delta);
		void prev (uint32_t delta);
		uint32_t get_distance_from (Iterator const &o) const;

		bool is_end (void) const;
		bool is_start (void) const;

		// what an evil method !!
		uint8_t *peek_data (void);

		INL_EXPE void write_u8 (uint8_t  data);
		void write_u8 (uint8_t data, uint32_t len);
		void write_u16 (uint16_t data);
		void write_u32 (uint32_t data);
		void write_u64 (uint64_t data);
		INL_EXPE void write_hton_u16 (uint16_t data);
		void write_hton_u32 (uint32_t data);
		void write_hton_u64 (uint64_t data);
		void write (uint8_t const*buffer, uint16_t size);
		void write (Iterator start, Iterator end);

		uint8_t  read_u8 (void);
		uint16_t read_u16 (void);
		uint32_t read_u32 (void);
		uint64_t read_u64 (void);
		uint16_t read_ntoh_u16 (void);
		uint32_t read_ntoh_u32 (void);
		uint64_t read_ntoh_u64 (void);
		void read (uint8_t *buffer, uint16_t size);
	private:
		friend class Buffer;
		Iterator (uint8_t *start, uint8_t *end, uint8_t *current);
		uint8_t *m_start;
		uint8_t *m_end;
		uint8_t *m_current;
	};

	Buffer ();
	~Buffer ();

	uint32_t get_size (void) const;

	/* after a call to any of these methods, 
	 * any iterator is invalidated.
	 */
	void add_at_start (uint32_t start);
	void add_at_end (uint32_t end);
	void remove_at_start (uint32_t start);
	void remove_at_end (uint32_t end);

	Buffer::Iterator begin (void) const;
	Buffer::Iterator end (void) const;

private:
	uint8_t *alloc_and_zero (uint32_t size) const;
	uint8_t *get_start (void) const;

	static uint32_t get_prefered_start (void);
	static uint32_t get_prefered_size (void);
	static void record_buffer_stats (uint32_t start, uint32_t end);
	static uint32_t m_total_start;
	static uint32_t m_total_end;

	uint32_t m_size;
	uint32_t m_start;
	uint8_t *m_data;
	uint32_t m_real_size;
	uint32_t m_total_added_start;
	uint32_t m_total_added_end;
};


}; // namespace yans

#ifdef INL_EXPE
#include <cassert>
namespace yans {
void 
Buffer::Iterator::write_u8  (uint8_t  data)
{
	assert (m_current + 1 <= m_end);
	*m_current = data;
}
void 
Buffer::Iterator::write_hton_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_end);
	*(m_current+0) = (data >> 8) & 0xff;
	*(m_current+1) = (data >> 0) & 0xff;
}
}
#endif

#endif /* BUFFER_H */
