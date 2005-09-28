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
WriteBuffer::write_u16 (uint16_t data)
{
	ensure_write_room_left (2);
	m_buffer[m_write] = (data >> 8) & 0xff;
	m_write++;
	m_buffer[m_write] = (data >> 0) & 0xff;
	m_write++;
}

void 
WriteBuffer::write_u32 (uint32_t data)
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
void 
WriteBuffer::write_htons_u32 (uint32_t data)
{
	ensure_write_room_left (4);
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
ReadBuffer::read_u16 (void)
{
	assert (is_read_room_left (2));
	uint16_t retval = m_buffer[m_read] << 8;
	m_read++;
	retval |= m_buffer[m_read];
	m_read++;
	return retval;
}

uint32_t 
ReadBuffer::read_u32 (void)
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


uint32_t 
ReadBuffer::read_nstoh_u32 (void)
{
	assert (is_read_room_left (4));
	return 0;
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
	WriteBuffer *buffer = new WriteBuffer (1);
	buffer->write_u8 (0x66);
	if (buffer->get_written_size () != 1) {
		ok = false;
	}
	if (buffer->peek_data () [0] != 0x66) {
		ok = false;
	}
	buffer->reset ();
	buffer->write_u8 (0x67);
	if (buffer->get_written_size () != 1) {
		ok = false;
	}
	if (buffer->peek_data () [0] != 0x67) {
		ok = false;
	}
	buffer->write_u16 (0x6568);
	if (buffer->get_written_size () != 3) {
		ok = false;
	}
	if (buffer->peek_data () [0] != 0x67) {
		ok = false;
	}
	if (buffer->peek_data () [0] != 0x65) {
		ok = false;
	}
	if (buffer->peek_data () [0] != 0x68) {
		ok = false;
	}
#if 0
	buffer->reset ();
	buffer->write_u16 (0x6369);
	if (buffer->read_u16 () != 0x6369) {
		ok = false;
	}
	buffer->write_u32 (0xdeadbeaf);
	if (buffer->read_u32 () != 0xdeadbeaf) {
		ok = false;
	}
	buffer->write_htons_u32 (0xdeadbeaf);
	if (buffer->read_nstoh_u32 () != 0xdeadbeaf) {
		ok = false;
	}
#endif
	return ok;
}

#endif /* RUN_SELF_TESTS */
