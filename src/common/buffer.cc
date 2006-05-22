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
	m_size = 0xdeadbeaf;

	record_buffer_stats (m_total_added_start, m_total_added_end);
	m_total_added_start = 0xdeadbeaf;
	m_total_added_end = 0xdeadbeaf;
}

void 
Buffer::cleanup (void)
{
	remove_at_start (get_size ());
	record_buffer_stats (m_total_added_start, m_total_added_end);
}
void 
Buffer::reset (void)
{
	m_start = get_prefered_start ();
	m_size = 0;
	if (m_real_size != get_prefered_size ()) {
		delete [] m_data;
		if (get_prefered_size () > 0) {
			TRACE ("allocating size="<<get_prefered_size ()<<", at="<<m_start);
			m_data = alloc_and_zero (get_prefered_size ());
			m_real_size = get_prefered_size ();
		} else {
			m_data = 0;
			m_real_size = 0;
		}
	}
	m_total_added_start = 0;
	m_total_added_end = 0;
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
		return;
	}
	uint32_t new_size = m_size + start;
	if (new_size <= m_real_size) {
		memmove (m_data+start, get_start (), m_size);
		m_start = 0;
		m_size = new_size;
		return;
	}
	uint8_t *new_buffer = alloc_and_zero (new_size);

	memcpy (new_buffer+start, get_start (), m_size);

	delete [] m_data;
	m_data = new_buffer;
	m_real_size = new_size;

	m_start = 0;
	m_size = new_size;
}
void 
Buffer::add_at_end (uint32_t end)
{
	m_total_added_end += end;
	if (m_start + m_size + end < m_real_size) {
		m_size += end;
		return;
	}
	uint32_t new_size = m_size + end;
	if (new_size <= m_real_size) {
		uint32_t new_start = m_real_size - (m_size + end);
		memmove (m_data+new_start, m_data+m_start, m_size);
		m_start = new_start;
		m_size += end;
		return;
	}
	uint8_t *new_buffer = alloc_and_zero (new_size);

	memcpy (new_buffer, get_start (), m_size);

	delete [] m_data;
	m_data = new_buffer;
	m_real_size = new_size;

	m_start = 0;
	m_size = new_size;
}
void 
Buffer::remove_at_start (uint32_t start)
{
	if (m_size <= start) {
		m_start += m_size;
		m_size = 0;
		return;
	}
	m_start += start;
	m_size -= start;
}
void 
Buffer::remove_at_end (uint32_t end)
{
	if (m_size <= end) {
		m_size = 0;
		return;
	}
	m_size -= end;
}

uint8_t *
Buffer::alloc_and_zero (uint32_t size) const
{
	uint8_t *buffer = new uint8_t [size];
#ifndef NDEBUG
        memset (buffer, 0x33, size);
#endif
	return buffer;
}


Buffer::Iterator 
Buffer::begin (void) const
{
	uint8_t *start = get_start ();
	uint8_t *end = start + m_size;
	return Buffer::Iterator (start, end, start);
}
Buffer::Iterator 
Buffer::end (void) const
{
	uint8_t *start = get_start ();
	uint8_t *end = start + m_size;
	return Buffer::Iterator (start, end, end);
}

Buffer::Iterator::Iterator ()
	: m_start (0), m_end (0), m_current (0)
{}
Buffer::Iterator::Iterator (uint8_t *start, uint8_t *end, uint8_t *current)
	: m_start (start), m_end (end), m_current (current)
{}

Buffer::Iterator::Iterator (Iterator const&o)
	: m_start (o.m_start), m_end (o.m_end), m_current (o.m_current)
{}
Buffer::Iterator &
Buffer::Iterator::operator = (Iterator const &o)
{
	m_start = o.m_start;
	m_end = o.m_end;
	m_current = o.m_current;
	return *this;
}
void 
Buffer::Iterator::next (void)
{
	assert (m_current + 1 <= m_end);
	m_current++;
}
void 
Buffer::Iterator::prev (void)
{
	assert (m_current - 1 >= m_start);
	m_current--;
}
void 
Buffer::Iterator::next (uint32_t delta)
{
	assert (m_current + delta <= m_end);
	m_current += delta;
}
void 
Buffer::Iterator::prev (uint32_t delta)
{
	assert (m_current - delta >= m_start);
	m_current -= delta;
}
uint32_t
Buffer::Iterator::get_distance_from (Iterator const &o) const
{
	assert (m_start == o.m_start);
	assert (m_end == o.m_end);
	unsigned long int start = reinterpret_cast<unsigned long int> (m_current);
	unsigned long int end = reinterpret_cast<unsigned long int> (o.m_current);
	return end - start;
}

bool 
Buffer::Iterator::is_end (void) const
{
	return m_current == m_end;
}
bool 
Buffer::Iterator::is_start (void) const
{
	return m_current == m_start;
}

uint8_t *
Buffer::Iterator::peek_data (void)
{
	return m_current;
}

void 
Buffer::Iterator::write (Iterator start, Iterator end)
{
	assert (start.m_current <= end.m_current);
	assert (start.m_start == end.m_start);
	assert (start.m_end == end.m_end);
	unsigned long int i_end = reinterpret_cast<unsigned long int> (end.m_current);
	unsigned long int i_start = reinterpret_cast<unsigned long int> (start.m_current);
	unsigned long int i_size = i_end - i_start;
	assert (m_current + i_size <= m_end);
	memcpy (m_current, start.m_current, i_size);
	m_current += i_size;
}

void 
Buffer::Iterator::write_u8 (uint8_t  data, uint32_t len)
{
	assert (m_current + len <= m_end);
	memset (m_current, data, len);
	m_current += len;
}

#ifndef INL_EXPE
void 
Buffer::Iterator::write_u8  (uint8_t  data)
{
	assert (m_current + 1 <= m_end);
	*m_current = data;
	m_current++;
}
#endif
void 
Buffer::Iterator::write_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_end);
	uint16_t *buffer = (uint16_t *)m_current;
	*buffer = data;
	m_current += 2;
}
void 
Buffer::Iterator::write_u32 (uint32_t data)
{
	assert (m_current + 4 <= m_end);
	uint32_t *buffer = (uint32_t *)m_current;
	*buffer = data;
	m_current += 4;
}
void 
Buffer::Iterator::write_u64 (uint64_t data)
{
	assert (m_current + 8 <= m_end);
	uint64_t *buffer = (uint64_t *)m_current;
	*buffer = data;
	m_current += 8;
}
#ifndef INL_EXPE
void 
Buffer::Iterator::write_hton_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_end);
	*(m_current+0) = (data >> 8) & 0xff;
	*(m_current+1) = (data >> 0) & 0xff;
	m_current += 2;
}
#endif
void 
Buffer::Iterator::write_hton_u32 (uint32_t data)
{
	assert (m_current + 4 <= m_end);
	*(m_current+0) = (data >> 24) & 0xff;
	*(m_current+1) = (data >> 16) & 0xff;
	*(m_current+2) = (data >> 8) & 0xff;
	*(m_current+3) = (data >> 0) & 0xff;
	m_current += 4;
}
void 
Buffer::Iterator::write_hton_u64 (uint64_t data)
{
	assert (m_current + 8 <= m_end);
	*(m_current+0) = (data >> 56) & 0xff;
	*(m_current+1) = (data >> 48) & 0xff;
	*(m_current+2) = (data >> 40) & 0xff;
	*(m_current+3) = (data >> 32) & 0xff;
	*(m_current+4) = (data >> 24) & 0xff;
	*(m_current+5) = (data >> 16) & 0xff;
	*(m_current+6) = (data >> 8) & 0xff;
	*(m_current+7) = (data >> 0) & 0xff;
	m_current += 8;
}
void 
Buffer::Iterator::write (uint8_t const*buffer, uint16_t size)
{
	assert (m_current + size <= m_end);
	memcpy (m_current, buffer, size);
	m_current += size;
}

uint8_t  
Buffer::Iterator::read_u8 (void)
{
	assert (m_current + 1 <= m_end);
	return *m_current;
	m_current ++;
}
uint16_t 
Buffer::Iterator::read_u16 (void)
{
	assert (m_current + 2 <= m_end);
	uint16_t *buffer = reinterpret_cast<uint16_t *>(m_current);
	m_current += 2;
	return *buffer;
}
uint32_t 
Buffer::Iterator::read_u32 (void)
{
	assert (m_current + 4 <= m_end);
	uint32_t *buffer = reinterpret_cast<uint32_t *>(m_current);
	m_current += 4;
	return *buffer;
}
uint64_t 
Buffer::Iterator::read_u64 (void)
{
	assert (m_current + 8 <= m_end);
	uint64_t *buffer = reinterpret_cast<uint64_t *>(m_current);
	m_current += 8;
	return *buffer;
}
uint16_t 
Buffer::Iterator::read_ntoh_u16 (void)
{
	assert (m_current + 2 <= m_end);
	uint16_t retval = 0;
	retval |= static_cast<uint16_t> (m_current[0]) << 8;
	retval |= static_cast<uint16_t> (m_current[1]) << 0;
	m_current += 2;
	return retval;
}
uint32_t 
Buffer::Iterator::read_ntoh_u32 (void)
{
	assert (m_current + 4 <= m_end);
	uint32_t retval = 0;
	retval |= static_cast<uint32_t> (m_current[0]) << 24;
	retval |= static_cast<uint32_t> (m_current[1]) << 16;
	retval |= static_cast<uint32_t> (m_current[2]) << 8;
	retval |= static_cast<uint32_t> (m_current[3]) << 0;
	m_current += 4;
	return retval;
}
uint64_t 
Buffer::Iterator::read_ntoh_u64 (void)
{
	assert (m_current + 8 <= m_end);
	uint64_t retval = 0;
	retval |= static_cast<uint64_t> (m_current[0]) << 56;
	retval |= static_cast<uint64_t> (m_current[1]) << 48;
	retval |= static_cast<uint64_t> (m_current[2]) << 40;
	retval |= static_cast<uint64_t> (m_current[3]) << 32;
	retval |= static_cast<uint64_t> (m_current[4]) << 24;
	retval |= static_cast<uint64_t> (m_current[5]) << 16;
	retval |= static_cast<uint64_t> (m_current[6]) << 8;
	retval |= static_cast<uint64_t> (m_current[7]) << 0;
	m_current += 8;
	return retval;
}
void 
Buffer::Iterator::read (uint8_t *buffer, uint16_t size)
{
	assert (m_current + size <= m_end);
	memcpy (buffer, m_current, size);
	m_current += size;
}

}; // namespace yans


#ifdef RUN_SELF_TESTS

#include "test.h"
#include <iomanip>

namespace yans {

class BufferTest: public Test {
private:
bool
ensure_written_bytes (Buffer::Iterator i, uint32_t n, uint8_t array[])
{
	bool success = true;
	uint8_t *expected = array;
	uint8_t *got;
	got = i.peek_data ();
	for (uint32_t j = 0; j < n; j++) {
		if (got[j] != expected[j]) {
			success = false;
		}
	}
	if (!success) {
		failure () << "Buffer -- ";
		failure () << "expected: ";
		failure () << n << " ";
		failure ().setf (std::ios::hex, std::ios::basefield);
		for (uint32_t j = 0; j < n; j++) {
			failure () << (uint16_t)expected[j] << " ";
		}
		failure ().setf (std::ios::dec, std::ios::basefield);
		failure () << "got: ";
		failure ().setf (std::ios::hex, std::ios::basefield);
		for (uint32_t j = 0; j < n; j++) {
			failure () << (uint16_t)got[j] << " ";
		}
		failure () << std::endl;
	}
	return success;
}

/* Note: works only when variadic macros are
 * available which is the case for gcc.
 * XXX
 */
#define ENSURE_WRITTEN_BYTES(iterator, n, ...) \
{ \
	uint8_t bytes[] = {__VA_ARGS__}; \
	if (!ensure_written_bytes (iterator, n , bytes)) { \
		ok = false; \
	} \
}

public:
virtual bool
run_tests (void)
{
	bool ok = true;
	Buffer *buffer = new Buffer ();
	Buffer::Iterator i;
	buffer->add_at_start (6);
	i = buffer->begin ();
	i.write_u8 (0x66);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 1, 0x66);
	i = buffer->begin ();
	i.write_u8 (0x67);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 1, 0x67);
	i.write_hton_u16 (0x6568);
	i = buffer->begin ();
	ENSURE_WRITTEN_BYTES (buffer->begin (), 3, 0x67, 0x65, 0x68);
	i.write_hton_u16 (0x6369);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 3, 0x63, 0x69, 0x68);
	i.write_hton_u32 (0xdeadbeaf);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 6, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf);
	buffer->add_at_start (2);
	i = buffer->begin ();
	i.write_u16 (0);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 8, 0, 0, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf);
	buffer->add_at_end (2);
	i = buffer->begin ();
	i.next (8);
	i.write_u16 (0);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 10, 0, 0, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf, 0, 0);
	buffer->remove_at_start (3);
	i = buffer->begin ();
	ENSURE_WRITTEN_BYTES (buffer->begin (), 7, 0x69, 0xde, 0xad, 0xbe, 0xaf, 0, 0);
	buffer->remove_at_end (4);
	i = buffer->begin ();
	ENSURE_WRITTEN_BYTES (buffer->begin (), 3, 0x69, 0xde, 0xad);
	buffer->add_at_start (1);
	i = buffer->begin ();
	i.write_u8 (0xff);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 4, 0xff, 0x69, 0xde, 0xad);
	buffer->add_at_end (1);
	i = buffer->begin ();
	i.next (4);
	i.write_u8 (0xff);
	i.prev (2);
	uint16_t saved = i.read_u16 ();
	i.prev (2);
	i.write_hton_u16 (0xff00);
	i.prev (2);
	if (i.read_ntoh_u16 () != 0xff00) {
		ok = false;
	}
	i.prev (2);
	i.write_u16 (saved);
	ENSURE_WRITTEN_BYTES (buffer->begin (), 5, 0xff, 0x69, 0xde, 0xad, 0xff);
	delete buffer;
	return ok;
}

BufferTest ()
	: Test ("Buffer") {}

};

static BufferTest g_buffer_test;

}; // namespace yans

#endif /* RUN_SELF_TESTS */


