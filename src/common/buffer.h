/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include <vector>

namespace yans {

/**
 * \brief automatically resized byte buffer
 *
 * This represents a buffer of bytes. Its size is
 * automatically adjusted to hold any data prepended
 * or appended by the user. Its implementation is optimized
 * to ensure that the number of buffer resizes is minimized,
 * by creating new Buffers of the maximum size ever used.
 * The correct maximum size is learned at runtime during use by 
 * recording the maximum size of each packet.
 */
class Buffer {
public:
	/**
	 * \brief iterator in a Buffer instance
	 */
	class Iterator {
	public:
		inline Iterator ();
		inline Iterator (Iterator const&o);
		inline Iterator &operator = (Iterator const &o);
		/**
		 * go forward by one byte
		 */
		inline void next (void);
		/**
		 * go backward by one byte
		 */
		inline void prev (void);
		/**
		 * \param delta number of bytes to go forward
		 */
		inline void next (uint32_t delta);
		/**
		 * \param delta number of bytes to go backward
		 */
		inline void prev (uint32_t delta);
		/**
		 * \param o the second iterator
		 * \return number of bytes included between the two iterators
		 *
		 * This method works only if the two iterators point
		 * to the same underlying buffer. Debug builds ensure
		 * this with an assert.
		 */
		inline uint32_t get_distance_from (Iterator const &o) const;

		/**
		 * \return true if this iterator points to the end of the byte array.
		 *         false otherwise.
		 */
		inline bool is_end (void) const;
		/**
		 * \return true if this iterator points to the start of the byte array.
		 *         false otherwise.
		 */
		inline bool is_start (void) const;

		/**
		 * \return a pointer to the current position in the
		 *         internal buffer
		 *
		 * Please, try to never ever use this method. It is really
		 * evil and is present only for a few specific uses.
		 */
		inline uint8_t *peek_data (void);

		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by one byte.
		 */
		inline void write_u8 (uint8_t  data);
		/**
		 * \param data data to write in buffer
		 * \param len number of times data must be written in buffer
		 *
		 * Write the data in buffer len times and avance the iterator position
		 * by len byte.
		 */
		inline void write_u8 (uint8_t data, uint32_t len);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by two bytes. The format of the data written in the byte
		 * buffer is non-portable. We only ensure that read_u16 will
		 * return exactly what we wrote with write_u16 if the program
		 * is run on the same machine.
		 */
		inline void write_u16 (uint16_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by four bytes. The format of the data written in the byte
		 * buffer is non-portable. We only ensure that read_u32 will
		 * return exactly what we wrote with write_u32 if the program
		 * is run on the same machine.
		 */
		inline void write_u32 (uint32_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by eight bytes. The format of the data written in the byte
		 * buffer is non-portable. We only ensure that read_u64 will
		 * return exactly what we wrote with write_u64 if the program
		 * is run on the same machine.
		 */
		inline void write_u64 (uint64_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by two bytes. The data is written in network order and the
		 * input data is expected to be in host order.
		 */
		inline void write_hton_u16 (uint16_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by four bytes. The data is written in network order and the
		 * input data is expected to be in host order.
		 */
		inline void write_hton_u32 (uint32_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by eight bytes. The data is written in network order and the
		 * input data is expected to be in host order.
		 */
		inline void write_hton_u64 (uint64_t data);
		/**
		 * \param buffer a byte buffer to copy in the internal buffer.
		 * \param size number of bytes to copy.
		 *
		 * Write the data in buffer and avance the iterator position
		 * by size bytes.
		 */
		inline void write (uint8_t const*buffer, uint16_t size);
		/**
		 * \param start the start of the data to copy
		 * \param end the end of the data to copy
		 *
		 * Write the data delimited by start and end in internal buffer 
		 * and avance the iterator position by the number of bytes
		 * copied.
		 * The input interators _must_ not point to the same Buffer as
		 * we do to avoid overlapping copies. This is enforced 
		 * in debug builds by asserts.
		 */
		inline void write (Iterator start, Iterator end);

		/**
		 * \return the byte read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 */
		inline uint8_t  read_u8 (void);
		/**
		 * \return the two bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in the format written by write_u16.
		 */
		inline uint16_t read_u16 (void);
		/**
		 * \return the four bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in the format written by write_u32.
		 */
		inline uint32_t read_u32 (void);
		/**
		 * \return the eight bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in the format written by write_u64.
		 */
		inline uint64_t read_u64 (void);
		/**
		 * \return the two bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in network format and return in host format.
		 */
		inline uint16_t read_ntoh_u16 (void);
		/**
		 * \return the four bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in network format and return in host format.
		 */
		inline uint32_t read_ntoh_u32 (void);
		/**
		 * \return the eight bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in network format and return in host format.
		 */
		inline uint64_t read_ntoh_u64 (void);
		/**
		 * \param buffer buffer to copy data into
		 * \param size number of bytes to copy
		 *
		 * Copy size bytes of data from the internal buffer to the
		 * input buffer and avance the Iterator by the number of
		 * bytes read.
		 */
		inline void read (uint8_t *buffer, uint16_t size);
	private:
		friend class Buffer;
		inline Iterator (uint8_t *start, uint8_t *end, uint8_t *current);
		uint8_t *m_start;
		uint8_t *m_end;
		uint8_t *m_current;
	};

	/**
	 * \return the number of bytes stored in this buffer.
	 */
	inline uint32_t get_size (void) const;

	/**
	 * \param start size to reserve
	 *
	 * Add bytes at the start of the Buffer. The
	 * content of these bytes is undefined but debugging
	 * builds initialize them to 0x33.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	void add_at_start (uint32_t start);
	/**
	 * \param end size to reserve
	 *
	 * Add bytes at the end of the Buffer. The
	 * content of these bytes is undefined but debugging
	 * builds initialize them to 0x33.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	void add_at_end (uint32_t end);
	/**
	 * \param start size to remove
	 *
	 * Remove bytes at the start of the Buffer.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	inline void remove_at_start (uint32_t start);
	/**
	 * \param end size to remove
	 *
	 * Remove bytes at the end of the Buffer.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	inline void remove_at_end (uint32_t end);

	/**
	 * \return an Iterator which points to the
	 * start of this Buffer.
	 */
	inline Buffer::Iterator begin (void) const;
	/**
	 * \return an Iterator which points to the
	 * end of this Buffer.
	 */
	inline Buffer::Iterator end (void) const;

	inline Buffer (Buffer const &o);
	inline Buffer &operator = (Buffer const &o);
	inline Buffer ();
	inline Buffer (uint32_t data_size);
	inline ~Buffer ();
private:
	struct BufferData {
		uint32_t m_count;
		uint32_t m_size;
		uint32_t m_initial_start;
		uint32_t m_zero_area_size;
		uint32_t m_dirty_start;
		uint32_t m_dirty_size;
		uint8_t m_data[1];
	};
	typedef std::vector<struct Buffer::BufferData*> BufferDataList;

	inline uint8_t *get_start (void) const;
	static void recycle (struct Buffer::BufferData *data);
	static struct Buffer::BufferData *create (void);
	static struct Buffer::BufferData *allocate (uint32_t size, uint32_t start);
	static void deallocate (struct Buffer::BufferData *data);

	static BufferDataList m_free_list;
	static uint32_t m_max_total_add_start;
	static uint32_t m_max_total_add_end;

	struct BufferData *m_data;
	uint32_t m_start;
	uint32_t m_size;
};

}; // namespace yans

#include <cassert>

namespace yans {

Buffer::Buffer ()
	: m_data (Buffer::create ()),
	  m_start (m_max_total_add_start),
	  m_size (0)
{
	if (m_start > m_data->m_size) {
		m_start = 0;
	}
	assert (m_start <= m_data->m_size);
}

Buffer::Buffer (uint32_t data_size)
	: m_data (Buffer::create ()),
	  m_start (m_max_total_add_start),
	  m_size (0)
{
	if (m_start > m_data->m_size) {
		m_start = 0;
	}
	m_data->m_zero_area_size = data_size;
	assert (m_start <= m_data->m_size);
}


Buffer::Buffer (Buffer const&o)
	: m_data (o.m_data),
	  m_start (o.m_start),
	  m_size (o.m_size)
{
	m_data->m_count++;
	assert (m_start <= m_data->m_size);
}

Buffer &
Buffer::operator = (Buffer const&o)
{
	if (m_data != o.m_data) {
		// not assignment to self.
		m_data->m_count--;
		if (m_data->m_count == 0) {
			recycle (m_data);
		}
		m_data = o.m_data;
		m_data->m_count++;
	}
	m_start = o.m_start;
	m_size = o.m_size;
	assert (m_start <= m_data->m_size);
	return *this;
}

Buffer::~Buffer ()
{
	m_data->m_count--;
	if (m_data->m_count == 0) {
		recycle (m_data);
	}
}


uint8_t *
Buffer::get_start (void) const
{
	return m_data->m_data + m_start;
}

uint32_t 
Buffer::get_size (void) const
{
	return m_size;
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
	assert (start.m_start != m_start);
	assert (start.m_end != m_end);
	assert (end.m_start != m_start);
	assert (end.m_end != m_end);
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
void 
Buffer::Iterator::write_u8  (uint8_t  data)
{
	assert (m_current + 1 <= m_end);
	*m_current = data;
	m_current++;
}
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
void 
Buffer::Iterator::write_hton_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_end);
	*(m_current+0) = (data >> 8) & 0xff;
	*(m_current+1) = (data >> 0) & 0xff;
	m_current += 2;
}
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
	uint8_t data = *m_current;
	m_current++;
	return data;
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


#endif /* BUFFER_H */
