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


class Buffer {
public:
	Buffer ();
	Buffer (uint32_t initial_size);
	~Buffer ();

	uint8_t *peek_data (void);
	uint32_t get_used_size (void) const;
	uint32_t get_size (void) const;
	uint32_t get_current (void) const;

	void seek (uint32_t offset);
	void skip (int32_t delta);

	void write_u8  (uint8_t  data);
	void write_u16 (uint16_t data);
	void write_u32 (uint32_t data);
	void write (uint8_t const*buffer, uint16_t size);
	void write_hton_u16 (uint16_t data);
	void write_hton_u32 (uint32_t data);

	uint8_t  read_u8 (void);
	uint16_t read_u16 (void);
	uint32_t read_u32 (void);
	void read (uint8_t *buffer, uint16_t size);
	uint16_t read_ntoh_u16 (void);
	uint32_t read_ntoh_u32 (void);

private:
	void ensure_room_left (uint16_t needed);
	void ensure_size_is (uint32_t target);
	bool is_room_left (uint16_t needed);
	uint8_t *alloc_and_zero (uint32_t size);
	uint32_t m_size;
	uint32_t m_current;
	uint8_t *m_buffer;
};

class WriteBuffer {
public:
	WriteBuffer (uint32_t prefered_size);
	~WriteBuffer ();

	/* used to copy the data out of this write buffer. */
	uint8_t *peek_data (void);
	uint32_t get_written_size (void);
	void reset (void);

	void write_u8 (uint8_t data);
	void write (uint8_t const*buffer, uint16_t size);
	void write_hton_u16 (uint16_t data);
	void write_hton_u32 (uint32_t data);

private:
	void ensure_write_room_left (uint16_t needed);
	uint32_t m_size;
	uint32_t m_write;
	uint8_t *m_buffer;
};

class ReadBuffer {
public:
	ReadBuffer (uint32_t size);
	~ReadBuffer ();

	/* used to copy the data in this read buffer. */
	uint8_t *peek_data (void);

	uint8_t read_u8 (void);
	void read (uint8_t *buffer, uint16_t size);
	uint16_t read_ntoh_u16 (void);
	uint32_t read_ntoh_u32 (void);

private:
	bool is_read_room_left (uint16_t needed);
	uint32_t m_size;
	uint32_t m_read;
	uint8_t *m_buffer;
};

#ifdef RUN_SELF_TESTS
#include "test.h"
class BufferTest: public Test {
public:
	BufferTest (TestManager *manager);
	
	virtual bool run_tests (void);
private:
	bool ensure_written_bytes (WriteBuffer *buffer, 
				   uint32_t n, uint8_t array[]);
};
#endif /* RUN_SELF_TESTS */

#endif /* BUFFER_H */
