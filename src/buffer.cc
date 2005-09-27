/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "buffer.h"

#include <cassert>
#include <string.h>


Buffer::Buffer (uint32_t size)
{
	if (size == 0) {
		m_size = 16;
	} else {
		m_size = size;
	}
	m_buffer = new uint8_t [m_size];
}

Buffer::~Buffer ()
{
	delete [] m_buffer;
}

void
Buffer::ensure_write_room_left (uint8_t needed)
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
bool
Buffer::is_read_room_left (uint8_t needed)
{
	if ((m_write - m_read) > needed) {
		return true;
	} else {
		return false;
	}
}

void 
Buffer::write_u8 (uint8_t data)
{
	ensure_write_room_left (1);
	m_buffer[m_write] = data;
	m_write++;
}

uint8_t 
Buffer::read_u8 (void)
{
	assert (is_read_room_left (1));
	uint8_t retval = m_buffer[m_read];
	m_read++;
	return retval;
}

void 
Buffer::write_u16 (uint16_t data)
{
	ensure_write_room_left (2);
	m_buffer[m_write] = (data >> 8) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 0) & 0xff;
	m_write++;
}
uint16_t 
Buffer::read_u16 (void)
{
	assert (is_read_room_left (2));
	uint16_t retval = m_buffer[m_read] << 8;
	m_read++;
	retval |= m_buffer[m_read];
	m_read++;
	return retval;
}

void 
Buffer::write_u32 (uint32_t data)
{
	ensure_write_room_left (4);
	m_buffer[m_write] = (data >> 24) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 16) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 8)  & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 0)  & 0xff;
	m_write++;
}
uint32_t 
Buffer::read_u32 (void)
{
	assert (is_read_room_left (4));
	uint32_t retval = 0;
	retval |= m_buffer[m_read] << 24;
	m_read++;
	retval |= m_buffer[m_read] << 16;
	m_read++;
	retval |= m_buffer[m_read] << 8;
	m_read++;
	retval |= m_buffer[m_read];
	m_read++;
	return retval;
}


void 
Buffer::write_htons_u32 (uint32_t data)
{
	ensure_write_room_left (4);
}
uint32_t 
Buffer::read_nstoh_u32 (void)
{
	assert (is_read_room_left (4));
	return 0;
}


void 
Buffer::reset (void)
{
	m_write = 0;
	m_read = 0;
}
uint32_t 
Buffer::get_written_size (void)
{
	return m_write;
}
uint8_t *
Buffer::peek_data (void)
{
	return m_buffer;
}

#ifdef RUN_SELF_TESTS

#include "test.h"

BufferTest::BufferTest (TestManager *manager)
	: Test (manager)
{}

bool
BufferTest::run_tests (void)
{
	bool ok = true;
	Buffer *buffer = new Buffer (1);
	buffer->write_u8 (0x66);
	if (buffer->read_u8 () != 0x66) {
		ok = false;
	}
	buffer->reset ();
	buffer->write_u8 (0x67);
	if (buffer->read_u8 () != 0x67) {
		ok = false;
	}
	
	return ok;
}

#endif /* RUN_SELF_TESTS */
