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

#include "buffer.h"

#define TRACE_BUFFER 1

#ifdef TRACE_BUFFER
#include <iostream>
#include "simulator.h"
# define TRACE(x) \
std::cout << "BUFFER TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_BUFFER */
# define TRACE(format,...)
#endif /* TRACE_BUFFER */


#include <cassert>
#include <string.h>

namespace yans {


Buffer::Buffer ()
{
	m_current = 0;
	m_size = 512;
	m_buffer = alloc_and_zero (m_size);
}
Buffer::Buffer (uint32_t initial_size)
{
	m_current = 0;
	if (initial_size == 0) {
		m_size = 16;
	} else {
		m_size = initial_size;
	}
	m_buffer = alloc_and_zero (m_size);
}

Buffer::~Buffer ()
{
	delete [] m_buffer;
	m_buffer = (uint8_t *)0xdeadbeaf;
	m_current = 0xdeadbeaf;
	m_size = 0xdeadbeaf;
}

uint8_t *
Buffer::peek_data (void)
{
	return m_buffer;
}
uint32_t 
Buffer::get_used_size (void) const
{
	return m_current;
}
uint32_t 
Buffer::get_size (void) const
{
	return m_size;
}
uint32_t 
Buffer::get_current (void) const
{
	return m_current;
}

void 
Buffer::seek (uint32_t offset)
{
	ensure_size_is (offset);
	assert (offset <= m_size);
	m_current = offset;
}
void 
Buffer::skip (int32_t delta)
{
	if (delta < 0) {
		uint32_t rewind = -delta;
		if (m_current < rewind) {
			delta = -m_current;
		}
	}
	uint32_t new_current = m_current + delta;
	ensure_size_is (new_current);
	m_current = new_current;
}

void 
Buffer::write_u8 (uint8_t data)
{
	ensure_room_left (1);
	m_buffer[m_current] = data;
	m_current++;
}
void 
Buffer::write_u16 (uint16_t data)
{
	ensure_room_left (2);
	uint16_t *buffer = (uint16_t *)(m_buffer + m_current);
	*buffer = data;
	m_current += 2;
}
void 
Buffer::write_u32 (uint32_t data)
{
	ensure_room_left (4);
	uint32_t *buffer = (uint32_t *)(m_buffer + m_current);
	*buffer = data;
	m_current += 4;
}
void 
Buffer::write (uint8_t const *data, uint16_t size)
{
	ensure_room_left (size);
	memcpy (m_buffer+m_current, data, size);
	m_current += size;
}
void 
Buffer::write_hton_u16 (uint16_t data)
{
	ensure_room_left (2);
	m_buffer[m_current] = (data >> 8) & 0xff;
	m_current++;
	m_buffer[m_current] = (data >> 0) & 0xff;
	m_current++;
}

void 
Buffer::write_hton_u32 (uint32_t data)
{
	ensure_room_left (4);
	m_buffer[m_current] = (data >> 24)  & 0xff;
	m_current++;
	m_buffer[m_current] = (data >> 16)  & 0xff;
	m_current++;
	m_buffer[m_current] = (data >> 8) & 0xff;
	m_current++;
	m_buffer[m_current] = (data >> 0) & 0xff;
	m_current++;
}

uint8_t 
Buffer::read_u8 (void)
{
	assert (is_room_left (1));
	uint8_t retval = m_buffer[m_current];
	m_current++;
	return retval;
}
uint16_t 
Buffer::read_u16 (void)
{
	assert (is_room_left (2));
	uint16_t *buffer = (uint16_t *)(m_buffer + m_current);
	uint16_t retval = *buffer;
	m_current += 4;
	return retval;
}
uint32_t 
Buffer::read_u32 (void)
{
	assert (is_room_left (4));
	uint32_t *buffer = (uint32_t *)(m_buffer + m_current);
	uint32_t retval = *buffer;
	m_current += 4;
	return retval;
}
void 
Buffer::read (uint8_t *buffer, uint16_t size)
{
	assert (is_room_left (size));
	memcpy (buffer, m_buffer+m_current, size);
	m_current += size;
}

uint16_t 
Buffer::read_ntoh_u16 (void)
{
	assert (is_room_left (2));
	uint16_t retval = 0;
	retval |= m_buffer[m_current] << 8;
	m_current++;
	retval |= m_buffer[m_current] << 0;
	m_current++;
	return retval;
}

uint32_t 
Buffer::read_ntoh_u32 (void)
{
	assert (is_room_left (4));
	uint32_t retval = 0;
	retval |= m_buffer[m_current] << 24;
	m_current++;
	retval |= m_buffer[m_current] << 16;
	m_current++;
	retval |= m_buffer[m_current] << 8;
	m_current++;
	retval |= m_buffer[m_current] << 0;
	m_current++;
	return retval;
}

void
Buffer::ensure_room_left (uint16_t needed)
{
	ensure_size_is (m_current + needed);
}

void
Buffer::ensure_size_is (uint32_t target)
{
	if (target <= m_size) {
		return;
	}
	uint32_t new_size = m_size;
	assert (new_size > 0);
	while (new_size < target) {
		//TRACE ("new size: "<<new_size);
		new_size <<= 1;
	}
	uint8_t *new_data = alloc_and_zero (new_size);
	memcpy (new_data, m_buffer, m_current);
	delete [] m_buffer;
	m_buffer = new_data;
	m_size = new_size;
}

bool
Buffer::is_room_left (uint16_t needed)
{
	if ((m_size - m_current) >= needed) {
		return true;
	} else {
		return false;
	}
}

uint8_t *
Buffer::alloc_and_zero (uint32_t size)
{
	uint8_t *buffer = new uint8_t [size];
	memset (buffer, 0x33, size);
	return buffer;
}

}; // namespace yans


#ifdef RUN_SELF_TESTS

#include "test.h"
#include <iomanip>

namespace yans {

BufferTest::BufferTest (TestManager *manager)
	: Test (manager)
{}

bool
BufferTest::ensure_written_bytes (Buffer *buffer, uint32_t n, uint8_t array[])
{
	bool success = true;
	uint8_t *expected = array;
	uint8_t *got;
	if (buffer->get_current () != n) {
		success = false;
	}
	got = buffer->peek_data ();
	for (uint32_t i = 0; i < n; i++) {
		if (got[i] != expected[i]) {
			success = false;
		}
	}
	if (!success) {
		failure () << "Buffer -- ";
		failure () << "expected: ";
		failure () << n << " ";
		failure ().setf (std::ios::hex, std::ios::basefield);
		for (uint32_t i = 0; i < n; i++) {
			failure () << (uint16_t)expected[i] << " ";
		}
		failure ().setf (std::ios::dec, std::ios::basefield);
		failure () << "got: ";
		failure () << buffer->get_current () << " ";
		failure ().setf (std::ios::hex, std::ios::basefield);
		for (uint32_t i = 0; i < n; i++) {
			failure () << (uint16_t)got[i] << " ";
		}
		failure () << std::endl;
	}
	return success;
}

/* Note: works only when variadic macros are
 * available which is the case for gcc.
 * XXX
 */
#define ENSURE_WRITTEN_BYTES(buffer, n, ...) \
{ \
	uint8_t bytes[] = {__VA_ARGS__}; \
	if (!ensure_written_bytes (buffer, n , bytes)) { \
		ok = false; \
	} \
}

bool
BufferTest::run_tests (void)
{
	bool ok = true;
	Buffer *buffer = new Buffer (1);
	buffer->write_u8 (0x66);
	ENSURE_WRITTEN_BYTES (buffer, 1, 0x66);
	buffer->seek (0);
	buffer->write_u8 (0x67);
	ENSURE_WRITTEN_BYTES (buffer, 1, 0x67);
	buffer->write_hton_u16 (0x6568);
	ENSURE_WRITTEN_BYTES (buffer, 3, 0x67, 0x68, 0x65);
	buffer->seek (0);
	buffer->write_hton_u16 (0x6369);
	ENSURE_WRITTEN_BYTES (buffer, 2, 0x69, 0x63);
	buffer->write_hton_u32 (0xdeadbeaf);
	ENSURE_WRITTEN_BYTES (buffer, 6, 0x69, 0x63, 0xaf, 0xbe, 0xad, 0xde);
	return ok;
}

}; // namespace yans

#endif /* RUN_SELF_TESTS */


