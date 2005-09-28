/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "buffer.h"

#include <cassert>
#include <string.h>


WriteBuffer::WriteBuffer (uint32_t size)
{
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
WriteBuffer::ensure_write_room_left (uint8_t needed)
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
WriteBuffer::write_htons_u16 (uint16_t data)
{
	ensure_write_room_left (2);
	m_buffer[m_write] = (data >> 0) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 8) & 0xff;
	m_write++;
}

void 
WriteBuffer::write_htons_u32 (uint32_t data)
{
	ensure_write_room_left (4);
	m_buffer[m_write] = (data >> 0)  & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 8)  & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 16) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 24) & 0xff;
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
ReadBuffer::is_read_room_left (uint8_t needed)
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

uint16_t 
ReadBuffer::read_nstoh_u16 (void)
{
	assert (is_read_room_left (2));
	uint16_t retval = 0;
	retval |= m_buffer[m_read];
	m_read++;
	retval |= m_buffer[m_read] << 8;
	m_read++;
	return retval;
}

uint32_t 
ReadBuffer::read_nstoh_u32 (void)
{
	assert (is_read_room_left (4));
	uint32_t retval = 0;
	retval |= m_buffer[m_read];
	m_read++;
	retval |= m_buffer[m_read] << 8;
	m_read++;
	retval |= m_buffer[m_read] << 16;
	m_read++;
	retval |= m_buffer[m_read] << 24;
	m_read++;
	return retval;
}


#ifdef RUN_SELF_TESTS

#include "test.h"
#include <iomanip>

BufferTest::BufferTest (TestManager *manager)
	: Test (manager)
{}

void
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
		for (uint32_t i = 0; i < n; i++) {
			failure ().width (2);
			failure ().fill (0);
			failure () << std::ios::hex << array[i] << " ";
		}
		failure () << "got: ";
		failure () << buffer->get_written_size () << " ";
		for (uint32_t i = 0; i < n; i++) {
			failure ().width (2);
			failure ().fill (0);
			failure () << std::ios::hex << got[i] << " ";
		}
		failure () << std::endl;
	}
}

/* Note: works only when variadic macros are
 * available which is the case for gcc.
 * XXX
 */
#define ENSURE_WRITTEN_BYTES(buffer, n, ...) \
{ \
	uint8_t bytes[] = {__VA_ARGS__}; \
	ensure_written_bytes (buffer, n , bytes); \
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
	buffer->write_htons_u16 (0x6568);
	ENSURE_WRITTEN_BYTES (buffer, 3, 0x67, 0x68, 0x64);
	buffer->reset ();
	buffer->write_htons_u16 (0x6369);
	ENSURE_WRITTEN_BYTES (buffer, 2, 0x69, 0x63);
	buffer->write_htons_u32 (0xdeadbeaf);
	ENSURE_WRITTEN_BYTES (buffer, 6, 0x69, 0x63, 0xaf, 0xbe, 0xad, 0xde);
	return ok;
}

#endif /* RUN_SELF_TESTS */
