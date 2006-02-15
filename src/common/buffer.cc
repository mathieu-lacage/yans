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

#define noTRACE_BUFFER 1

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

uint32_t Buffer::m_total_start = 0;
uint32_t Buffer::m_total_end = 0;

Buffer::Buffer ()
{
	m_start = get_prefered_start ();
	m_size = 0;
	m_current = 0;

	if (get_prefered_size () > 0) {
		TRACE ("allocating size="<<get_prefered_size ()<<", at="<<m_start);
		m_data = alloc_and_zero (get_prefered_size ());
		m_real_size = get_prefered_size ();
	} else {
		m_data = 0;
		m_real_size = 0;
	}
	m_total_added_start = 0;
	m_total_added_end = 0;
}

Buffer::~Buffer ()
{
	if (m_data != 0) {
		delete [] m_data;
	}
	m_data = (uint8_t *)0xdeadbeaf;
	m_real_size = 0xdeadbeaf;

	m_start = 0xdeadbeaf;
	m_current = 0xdeadbeaf;
	m_size = 0xdeadbeaf;

	record_buffer_stats (m_total_added_start, m_total_added_end);
	m_total_added_start = 0xdeadbeaf;
	m_total_added_end = 0xdeadbeaf;
}

void
Buffer::record_buffer_stats (uint32_t start, uint32_t end)
{
	TRACE ("record stats start="<<start<<", end="<<end);
	if (start > m_total_start) {
		m_total_start = start;
	}
	if (end > m_total_end) {
		m_total_end = end;
	}
}
uint32_t
Buffer::get_prefered_start (void)
{
	return m_total_start;
}
uint32_t
Buffer::get_prefered_size (void)
{
	return m_total_start+m_total_end;
}

uint8_t *
Buffer::get_start (void) const
{
	return &m_data[m_start];
}

uint8_t *
Buffer::peek_data (void)
{
	return get_start ();
}
uint32_t 
Buffer::get_size (void) const
{
	return m_size;
}

void 
Buffer::add_at_start (uint32_t start)
{
	m_total_added_start += start;
	if (m_start >= start) {
		m_start -= start;
		m_size += start;
		m_current = 0;
		return;
	}
	uint32_t new_size = m_size + start;
	if (new_size <= m_real_size) {
		memmove (m_data+start, get_start (), m_size);
		m_start = 0;
		m_size = new_size;
		m_current = 0;
		return;
	}
	uint8_t *new_buffer = alloc_and_zero (new_size);

	memcpy (new_buffer+start, get_start (), m_size);

	delete [] m_data;
	m_data = new_buffer;
	m_real_size = new_size;

	m_start = 0;
	m_size = new_size;
	m_current = 0;
}
void 
Buffer::add_at_end (uint32_t end)
{
	m_total_added_end += end;
	if (m_start + m_size + end < m_real_size) {
		m_size += end;
		m_current = 0;
		return;
	}
	uint32_t new_size = m_size + end;
	if (new_size <= m_real_size) {
		uint32_t new_start = m_real_size - (m_size + end);
		memmove (m_data+new_start, m_data+m_start, m_size);
		m_start = new_start;
		m_size += end;
		m_current = 0;
		return;
	}
	uint8_t *new_buffer = alloc_and_zero (new_size);

	memcpy (new_buffer, get_start (), m_size);

	delete [] m_data;
	m_data = new_buffer;
	m_real_size = new_size;

	m_start = 0;
	m_size = new_size;
	m_current = 0;
}
void 
Buffer::remove_at_start (uint32_t start)
{
	if (m_size <= start) {
		m_start += m_size;
		m_size = 0;
		m_current = 0;
		return;
	}
	m_start += start;
	m_size -= start;
	m_current = 0;
}
void 
Buffer::remove_at_end (uint32_t end)
{
	if (m_size <= end) {
		m_size = 0;
		m_current = 0;
		return;
	}
	m_size -= end;
	m_current = 0;
}

uint32_t 
Buffer::get_current (void) const
{
	return m_current;
}

void 
Buffer::seek (uint32_t offset)
{
	assert (offset <= m_size);
	m_current = offset;
}
void 
Buffer::skip (int32_t delta)
{
	if (delta < 0) {
		assert (m_current >= ((uint32_t)-delta));
	}
	uint32_t new_current = m_current + delta;
	assert (new_current <= m_size);
	m_current = new_current;
}

void 
Buffer::write_u8 (uint8_t data)
{
	assert (m_current + 1 <= m_size);
	uint8_t *buffer = get_start ();
	buffer[m_current] = data;
	m_current++;
}
void 
Buffer::write_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_size);
	uint16_t *buffer = (uint16_t *)(get_start () + m_current);
	*buffer = data;
	m_current += 2;
}
void 
Buffer::write_u32 (uint32_t data)
{
	assert (m_current + 4 <= m_size);
	uint32_t *buffer = (uint32_t *)(get_start () + m_current);
	*buffer = data;
	m_current += 4;
}
void 
Buffer::write (uint8_t const *data, uint16_t size)
{
	assert (m_current + size <= m_size);
	memcpy (get_start ()+m_current, data, size);
	m_current += size;
}
void 
Buffer::write_hton_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_size);
	uint8_t *buffer = get_start ();
	buffer[m_current] = (data >> 8) & 0xff;
	m_current++;
	buffer[m_current] = (data >> 0) & 0xff;
	m_current++;
}

void 
Buffer::write_hton_u32 (uint32_t data)
{
	assert (m_current + 4 <= m_size);
	uint8_t *buffer = get_start ();
	buffer[m_current] = (data >> 24)  & 0xff;
	m_current++;
	buffer[m_current] = (data >> 16)  & 0xff;
	m_current++;
	buffer[m_current] = (data >> 8) & 0xff;
	m_current++;
	buffer[m_current] = (data >> 0) & 0xff;
	m_current++;
}

uint8_t 
Buffer::read_u8 (void)
{
	assert (m_current + 1 <= m_size);
	uint8_t *buffer = get_start ();
	uint8_t retval = buffer[m_current];
	m_current++;
	return retval;
}
uint16_t 
Buffer::read_u16 (void)
{
	assert (m_current + 2 <= m_size);
	uint16_t *buffer = (uint16_t *)(get_start () + m_current);
	uint16_t retval = *buffer;
	m_current += 2;
	return retval;
}
uint32_t 
Buffer::read_u32 (void)
{
	assert (m_current + 4 <= m_size);
	uint32_t *buffer = (uint32_t *)(get_start () + m_current);
	uint32_t retval = *buffer;
	m_current += 4;
	return retval;
}
void 
Buffer::read (uint8_t *buffer, uint16_t size)
{
	assert (m_current + size <= m_size);
	memcpy (buffer, get_start ()+m_current, size);
	m_current += size;
}

uint16_t 
Buffer::read_ntoh_u16 (void)
{
	assert (m_current + 2 <= m_size);
	uint16_t retval = 0;
	uint8_t *buffer = get_start ();
	retval |= buffer[m_current] << 8;
	m_current++;
	retval |= buffer[m_current] << 0;
	m_current++;
	return retval;
}

uint32_t 
Buffer::read_ntoh_u32 (void)
{
	assert (m_current + 4 <= m_size);
	uint32_t retval = 0;
	uint8_t *buffer = get_start ();
	retval |= buffer[m_current] << 24;
	m_current++;
	retval |= buffer[m_current] << 16;
	m_current++;
	retval |= buffer[m_current] << 8;
	m_current++;
	retval |= buffer[m_current] << 0;
	m_current++;
	return retval;
}

uint8_t *
Buffer::alloc_and_zero (uint32_t size) const
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

bool
BufferTest::ensure_written_bytes (Buffer *buffer, uint32_t n, uint8_t array[])
{
	bool success = true;
	uint8_t *expected = array;
	uint8_t *got;
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
	Buffer *buffer = new Buffer ();
	buffer->add_at_start (6);
	buffer->write_u8 (0x66);
	ENSURE_WRITTEN_BYTES (buffer, 1, 0x66);
	buffer->seek (0);
	buffer->write_u8 (0x67);
	ENSURE_WRITTEN_BYTES (buffer, 1, 0x67);
	buffer->write_hton_u16 (0x6568);
	ENSURE_WRITTEN_BYTES (buffer, 3, 0x67, 0x65, 0x68);
	buffer->seek (0);
	buffer->write_hton_u16 (0x6369);
	ENSURE_WRITTEN_BYTES (buffer, 2, 0x63, 0x69);
	buffer->write_hton_u32 (0xdeadbeaf);
	ENSURE_WRITTEN_BYTES (buffer, 6, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf);
	buffer->add_at_start (2);
	buffer->seek (0);
	buffer->write_u16 (0);
	ENSURE_WRITTEN_BYTES (buffer, 8, 0, 0, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf);
	buffer->add_at_end (2);
	buffer->seek (8);
	buffer->write_u16 (0);
	ENSURE_WRITTEN_BYTES (buffer, 10, 0, 0, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf, 0, 0);
	buffer->remove_at_start (3);
	ENSURE_WRITTEN_BYTES (buffer, 7, 0x69, 0xde, 0xad, 0xbe, 0xaf, 0, 0);
	buffer->remove_at_end (4);
	ENSURE_WRITTEN_BYTES (buffer, 3, 0x69, 0xde, 0xad);
	buffer->add_at_start (1);
	buffer->seek (0);
	buffer->write_u8 (0xff);
	ENSURE_WRITTEN_BYTES (buffer, 4, 0xff, 0x69, 0xde, 0xad);
	buffer->add_at_end (1);
	buffer->seek (4);
	buffer->write_u8 (0xff);
	buffer->skip (-2);
	uint16_t saved = buffer->read_u16 ();
	buffer->skip (-2);
	buffer->write_hton_u16 (0xff00);
	buffer->skip (-2);
	if (buffer->read_ntoh_u16 () != 0xff00) {
		ok = false;
	}
	buffer->skip (-2);
	buffer->write_u16 (saved);
	ENSURE_WRITTEN_BYTES (buffer, 4, 0xff, 0x69, 0xde, 0xad, 0xff);
	delete buffer;
	return ok;
}

}; // namespace yans

#endif /* RUN_SELF_TESTS */


