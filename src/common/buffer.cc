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

#include <cassert>
#include <string.h>


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
	delete m_buffer;
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
	assert (offset < m_size);
	m_current = offset;
}
void 
Buffer::skip (int32_t delta)
{
	if (delta < 0) {
		uint32_t rewind = -delta;
		assert (m_current > rewind);
	} else {
		assert (is_room_left (delta));
	}
	m_current += delta;
}

void 
Buffer::write_u8 (uint8_t data)
{
	ensure_room_left (1);
	m_buffer[m_current] = data;
	m_current++;
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
	if (m_size > (m_current + needed)) {
		return;
	}
	uint32_t new_size = m_size;
	assert (new_size > 0);
	while (new_size < (m_current + needed)) {
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



WriteBuffer::WriteBuffer (uint32_t size)
{
	m_write = 0;
	if (size == 0) {
		m_size = 16;
	} else {
		m_size = size;
	}
	m_buffer = new uint8_t [m_size];
}

WriteBuffer::~WriteBuffer ()
{
	delete [] m_buffer;
}

uint8_t *
WriteBuffer::peek_data (void)
{
	return m_buffer;
}
uint32_t 
WriteBuffer::get_written_size (void)
{
	return m_write;
}

void 
WriteBuffer::reset (void)
{
	m_write = 0;
}

void
WriteBuffer::ensure_write_room_left (uint16_t needed)
{
	if (m_size > (m_write + needed)) {
		return;
	}
	uint32_t new_size = m_size;
	assert (new_size > 0);
	while (new_size < (m_write + needed)) {
		new_size <<= 1;
	}
	uint8_t *new_data = new uint8_t [new_size];
	memcpy (new_data, m_buffer, m_write);
	delete [] m_buffer;
	m_buffer = new_data;
	m_size = new_size;
}
void 
WriteBuffer::write_u8 (uint8_t data)
{
	ensure_write_room_left (1);
	m_buffer[m_write] = data;
	m_write++;
}
void 
WriteBuffer::write (uint8_t const *data, uint16_t size)
{
	ensure_write_room_left (size);
	memcpy (m_buffer+m_write, data, size);
	m_write += size;
}
void 
WriteBuffer::write_hton_u16 (uint16_t data)
{
	ensure_write_room_left (2);
	m_buffer[m_write] = (data >> 8) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 0) & 0xff;
	m_write++;
}

void 
WriteBuffer::write_hton_u32 (uint32_t data)
{
	ensure_write_room_left (4);
	m_buffer[m_write] = (data >> 24)  & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 16)  & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 8) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 0) & 0xff;
	m_write++;
}


ReadBuffer::ReadBuffer (uint32_t size)
{
	m_size = size;
	m_buffer = new uint8_t [m_size];
}
ReadBuffer::~ReadBuffer ()
{
	delete m_buffer;
	m_buffer = 0;
	m_size = 0;
}
uint8_t *
ReadBuffer::peek_data (void)
{
	return m_buffer;
}
bool
ReadBuffer::is_read_room_left (uint16_t needed)
{
	if ((m_size - m_read) >= needed) {
		return true;
	} else {
		return false;
	}
}


uint8_t 
ReadBuffer::read_u8 (void)
{
	assert (is_read_room_left (1));
	uint8_t retval = m_buffer[m_read];
	m_read++;
	return retval;
}
void 
ReadBuffer::read (uint8_t *buffer, uint16_t size)
{
	assert (is_read_room_left (size));
	memcpy (buffer, m_buffer+m_read, size);
	m_read += size;
}

uint16_t 
ReadBuffer::read_ntoh_u16 (void)
{
	assert (is_read_room_left (2));
	uint16_t retval = 0;
	retval |= m_buffer[m_read] << 8;
	m_read++;
	retval |= m_buffer[m_read] << 0;
	m_read++;
	return retval;
}

uint32_t 
ReadBuffer::read_ntoh_u32 (void)
{
	assert (is_read_room_left (4));
	uint32_t retval = 0;
	retval |= m_buffer[m_read] << 24;
	m_read++;
	retval |= m_buffer[m_read] << 16;
	m_read++;
	retval |= m_buffer[m_read] << 8;
	m_read++;
	retval |= m_buffer[m_read] << 0;
	m_read++;
	return retval;
}



#ifdef RUN_SELF_TESTS

#include "test.h"
#include <iomanip>

BufferTest::BufferTest (TestManager *manager)
	: Test (manager)
{}

bool
BufferTest::ensure_written_bytes (WriteBuffer *buffer, uint32_t n, uint8_t array[])
{
	bool success = true;
	uint8_t *expected = array;
	uint8_t *got;
	if (buffer->get_written_size () != n) {
		success = false;
	}
	got = buffer->peek_data ();
	for (uint32_t i = 0; i < n; i++) {
		if (got[i] != expected[i]) {
			success = false;
		}
	}
	if (!success) {
		failure () << "WriteBuffer -- ";
		failure () << "expected: ";
		failure () << n << " ";
		failure ().setf (std::ios::hex, std::ios::basefield);
		for (uint32_t i = 0; i < n; i++) {
			failure () << (uint16_t)expected[i] << " ";
		}
		failure ().setf (std::ios::dec, std::ios::basefield);
		failure () << "got: ";
		failure () << buffer->get_written_size () << " ";
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
	WriteBuffer *buffer = new WriteBuffer (1);
	buffer->write_u8 (0x66);
	ENSURE_WRITTEN_BYTES (buffer, 1, 0x66);
	buffer->reset ();
	buffer->write_u8 (0x67);
	ENSURE_WRITTEN_BYTES (buffer, 1, 0x67);
	buffer->write_hton_u16 (0x6568);
	ENSURE_WRITTEN_BYTES (buffer, 3, 0x67, 0x68, 0x65);
	buffer->reset ();
	buffer->write_hton_u16 (0x6369);
	ENSURE_WRITTEN_BYTES (buffer, 2, 0x69, 0x63);
	buffer->write_hton_u32 (0xdeadbeaf);
	ENSURE_WRITTEN_BYTES (buffer, 6, 0x69, 0x63, 0xaf, 0xbe, 0xad, 0xde);
	return ok;
}

#endif /* RUN_SELF_TESTS */
