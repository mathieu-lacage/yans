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
#ifndef GBUFFER_H
#define GBUFFER_H

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
class GBuffer {
public:
	/**
	 * \brief iterator in a GBuffer instance
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
		friend class GBuffer;
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
	inline void add_at_start (uint32_t start);
	/**
	 * \param end size to reserve
	 *
	 * Add bytes at the end of the Buffer. The
	 * content of these bytes is undefined but debugging
	 * builds initialize them to 0x33.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	inline void add_at_end (uint32_t end);
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
	inline GBuffer::Iterator begin (void) const;
	/**
	 * \return an Iterator which points to the
	 * end of this Buffer.
	 */
	inline GBuffer::Iterator end (void) const;

	inline GBuffer (GBuffer const &o);
	inline GBuffer &operator = (GBuffer const &o);
	inline GBuffer ();
	inline ~GBuffer ();
private:
	struct GBufferData {
		uint32_t m_count;
		uint32_t m_size;
		uint32_t m_dirty_start;
		uint32_t m_dirty_size;
		uint8_t m_data;
	};
	typedef std::vector<struct GBuffer::GBufferData*> GBufferDataList;

	inline uint8_t *get_start (void) const;
	inline void recycle (struct GBuffer::GBufferData *data);
	inline static struct GBuffer::GBufferData *create (void);
	inline static struct GBuffer::GBufferData *allocate (uint32_t size, uint32_t start);
	inline static void deallocate (struct GBuffer::GBufferData *data);

	static GBufferDataList m_free_list;
	static uint32_t m_prefered_size;
	static uint32_t m_prefered_start;

	struct GBufferData *m_data;
	uint32_t m_total_added_start;
	uint32_t m_total_added_end;
	uint32_t m_start;
	uint32_t m_size;
};

}; // namespace yans

#include <cassert>

namespace yans {

struct GBuffer::GBufferData *
GBuffer::allocate (uint32_t req_size, uint32_t req_start)
{
	uint32_t size = req_size - 1 + sizeof (struct GBuffer::GBufferData);
	uint8_t *b = new uint8_t [size];
	struct GBufferData *data = reinterpret_cast<struct GBuffer::GBufferData*>(b);
	data->m_size = req_size;
	data->m_dirty_start = req_start;
	data->m_dirty_size = 0;
	data->m_count = 1;
	return data;
}

void
GBuffer::deallocate (struct GBuffer::GBufferData *data)
{
	uint8_t *buf = reinterpret_cast<uint8_t *> (data);
	delete [] buf;
}

void
GBuffer::recycle (struct GBuffer::GBufferData *data)
{
	/* update buffer statistics */
	if (m_total_added_start > GBuffer::m_prefered_start) {
		GBuffer::m_prefered_start = m_total_added_start;
	}
	uint32_t cur_prefered_end = GBuffer::m_prefered_size - GBuffer::m_prefered_start;
	uint32_t prefered_end;
	if (m_total_added_end > cur_prefered_end) {
		prefered_end = m_total_added_end;
	} else {
		prefered_end = cur_prefered_end;
	}
	GBuffer::m_prefered_size = GBuffer::m_prefered_start + prefered_end;
	/* feed into free list */
	if (GBuffer::m_free_list.size () > 1000) {
	GBuffer::deallocate (data);
} else {
	GBuffer::m_free_list.push_back (data);
 }
}

GBuffer::GBufferData *
GBuffer::create (void)
{
	if (!GBuffer::m_free_list.empty ()) {
		struct GBuffer::GBufferData *data = GBuffer::m_free_list.back ();
		GBuffer::m_free_list.pop_back ();
		if (GBuffer::m_prefered_start > data->m_size) {
			data->m_dirty_start = 0;
		} else {
			data->m_dirty_start = GBuffer::m_prefered_start;
		}
		data->m_dirty_size = 0;
		data->m_count = 1;
		return data;
	} else {
		struct GBuffer::GBufferData *data = GBuffer::allocate (GBuffer::m_prefered_size, 
								       GBuffer::m_prefered_start);
		return data;
	}
}

GBuffer::GBuffer ()
	: m_data (GBuffer::create ()),
	  m_total_added_start (0),
	  m_total_added_end (0),
	  m_start (GBuffer::m_prefered_start),
	  m_size (0)
{
	if (m_start > m_data->m_size) {
		m_start = 0;
	}
}

GBuffer::GBuffer (GBuffer const&o)
	: m_data (o.m_data),
	  m_total_added_start (0),
	  m_total_added_end (0),
	  m_start (o.m_start),
	  m_size (o.m_size)
{
	m_data->m_count++;
}

GBuffer &
GBuffer::operator = (GBuffer const&o)
{
	m_data->m_count--;
	if (m_data->m_count == 0) {
		recycle (m_data);
	}
	m_data = o.m_data;
	m_total_added_start = o.m_total_added_start;
	m_total_added_end = o.m_total_added_end;
	m_start = o.m_start;
	m_size = o.m_size;
	m_data->m_count++;
	return *this;
}

GBuffer::~GBuffer ()
{
	m_data->m_count--;
	if (m_data->m_count == 0) {
		recycle (m_data);
	}
}


uint8_t *
GBuffer::get_start (void) const
{
	uint8_t *buf = &m_data->m_data;
	return &buf[m_start];
}

uint32_t 
GBuffer::get_size (void) const
{
	return m_size;
}

void 
GBuffer::add_at_start (uint32_t start)
{
	m_total_added_start += start;
	if (m_start >= start) {
		/* plenty of extra space in the buffer */
		if (m_start > m_data->m_dirty_start && m_data->m_count > 1) {
			/* the buffer is dirty */
			m_data->m_count--;
			goto alloc_new_buffer;
		}
		m_start -= start;
		m_size += start;
		m_data->m_dirty_start = m_start;
		return;
	}
 alloc_new_buffer:
	uint32_t new_size = m_size + start;
	struct GBuffer::GBufferData *new_data = GBuffer::allocate (new_size, 0);
	uint8_t *buf = &new_data->m_data;
	memcpy (buf+start, get_start (), m_size);
	m_data->m_count--;
	if (m_data->m_count == 0) {
		GBuffer::deallocate (m_data);
	}
	m_data = new_data;
	m_size = new_size;
	m_start = 0;
}
void 
GBuffer::add_at_end (uint32_t end)
{
	m_total_added_end += end;
	if (m_start + m_size + end < m_data->m_size) {
		/* plenty of extra space in the buffer */
		if ((m_start + m_size) < (m_data->m_dirty_start + m_data->m_dirty_size) &&
		    m_data->m_count > 1) {
			/* the buffer is dirty */
			m_data->m_count--;
			goto alloc_new_buffer;
		}
		m_size += end;
		return;
	}
 alloc_new_buffer:
	uint32_t new_size = m_size + end;
	struct GBuffer::GBufferData *new_data = GBuffer::allocate (new_size, 0);
	memcpy (&new_data->m_data, get_start (), m_size);
	m_data->m_count--;
	if (m_data->m_count == 0) {
		GBuffer::deallocate (m_data);
	}
	m_data = new_data;
	m_size = new_size;
	m_start = 0;
}
void 
GBuffer::remove_at_start (uint32_t start)
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
GBuffer::remove_at_end (uint32_t end)
{
	if (m_size <= end) {
		m_size = 0;
		return;
	}
	m_size -= end;
}

GBuffer::Iterator 
GBuffer::begin (void) const
{
	uint8_t *start = get_start ();
	uint8_t *end = start + m_size;
	return GBuffer::Iterator (start, end, start);
}
GBuffer::Iterator 
GBuffer::end (void) const
{
	uint8_t *start = get_start ();
	uint8_t *end = start + m_size;
	return GBuffer::Iterator (start, end, end);
}


GBuffer::Iterator::Iterator ()
	: m_start (0), m_end (0), m_current (0)
{}
GBuffer::Iterator::Iterator (uint8_t *start, uint8_t *end, uint8_t *current)
	: m_start (start), m_end (end), m_current (current)
{}

GBuffer::Iterator::Iterator (Iterator const&o)
	: m_start (o.m_start), m_end (o.m_end), m_current (o.m_current)
{}
GBuffer::Iterator &
GBuffer::Iterator::operator = (Iterator const &o)
{
	m_start = o.m_start;
	m_end = o.m_end;
	m_current = o.m_current;
	return *this;
}
void 
GBuffer::Iterator::next (void)
{
	assert (m_current + 1 <= m_end);
	m_current++;
}
void 
GBuffer::Iterator::prev (void)
{
	assert (m_current - 1 >= m_start);
	m_current--;
}
void 
GBuffer::Iterator::next (uint32_t delta)
{
	assert (m_current + delta <= m_end);
	m_current += delta;
}
void 
GBuffer::Iterator::prev (uint32_t delta)
{
	assert (m_current - delta >= m_start);
	m_current -= delta;
}
uint32_t
GBuffer::Iterator::get_distance_from (Iterator const &o) const
{
	assert (m_start == o.m_start);
	assert (m_end == o.m_end);
	unsigned long int start = reinterpret_cast<unsigned long int> (m_current);
	unsigned long int end = reinterpret_cast<unsigned long int> (o.m_current);
	return end - start;
}

bool 
GBuffer::Iterator::is_end (void) const
{
	return m_current == m_end;
}
bool 
GBuffer::Iterator::is_start (void) const
{
	return m_current == m_start;
}

uint8_t *
GBuffer::Iterator::peek_data (void)
{
	return m_current;
}

void 
GBuffer::Iterator::write (Iterator start, Iterator end)
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
GBuffer::Iterator::write_u8 (uint8_t  data, uint32_t len)
{
	assert (m_current + len <= m_end);
	memset (m_current, data, len);
	m_current += len;
}
void 
GBuffer::Iterator::write_u8  (uint8_t  data)
{
	assert (m_current + 1 <= m_end);
	*m_current = data;
	m_current++;
}
void 
GBuffer::Iterator::write_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_end);
	uint16_t *buffer = (uint16_t *)m_current;
	*buffer = data;
	m_current += 2;
}
void 
GBuffer::Iterator::write_u32 (uint32_t data)
{
	assert (m_current + 4 <= m_end);
	uint32_t *buffer = (uint32_t *)m_current;
	*buffer = data;
	m_current += 4;
}
void 
GBuffer::Iterator::write_u64 (uint64_t data)
{
	assert (m_current + 8 <= m_end);
	uint64_t *buffer = (uint64_t *)m_current;
	*buffer = data;
	m_current += 8;
}
void 
GBuffer::Iterator::write_hton_u16 (uint16_t data)
{
	assert (m_current + 2 <= m_end);
	*(m_current+0) = (data >> 8) & 0xff;
	*(m_current+1) = (data >> 0) & 0xff;
	m_current += 2;
}
void 
GBuffer::Iterator::write_hton_u32 (uint32_t data)
{
	assert (m_current + 4 <= m_end);
	*(m_current+0) = (data >> 24) & 0xff;
	*(m_current+1) = (data >> 16) & 0xff;
	*(m_current+2) = (data >> 8) & 0xff;
	*(m_current+3) = (data >> 0) & 0xff;
	m_current += 4;
}
void 
GBuffer::Iterator::write_hton_u64 (uint64_t data)
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
GBuffer::Iterator::write (uint8_t const*buffer, uint16_t size)
{
	assert (m_current + size <= m_end);
	memcpy (m_current, buffer, size);
	m_current += size;
}

uint8_t  
GBuffer::Iterator::read_u8 (void)
{
	assert (m_current + 1 <= m_end);
	uint8_t data = *m_current;
	m_current++;
	return data;
}
uint16_t 
GBuffer::Iterator::read_u16 (void)
{
	assert (m_current + 2 <= m_end);
	uint16_t *buffer = reinterpret_cast<uint16_t *>(m_current);
	m_current += 2;
	return *buffer;
}
uint32_t 
GBuffer::Iterator::read_u32 (void)
{
	assert (m_current + 4 <= m_end);
	uint32_t *buffer = reinterpret_cast<uint32_t *>(m_current);
	m_current += 4;
	return *buffer;
}
uint64_t 
GBuffer::Iterator::read_u64 (void)
{
	assert (m_current + 8 <= m_end);
	uint64_t *buffer = reinterpret_cast<uint64_t *>(m_current);
	m_current += 8;
	return *buffer;
}
uint16_t 
GBuffer::Iterator::read_ntoh_u16 (void)
{
	assert (m_current + 2 <= m_end);
	uint16_t retval = 0;
	retval |= static_cast<uint16_t> (m_current[0]) << 8;
	retval |= static_cast<uint16_t> (m_current[1]) << 0;
	m_current += 2;
	return retval;
}
uint32_t 
GBuffer::Iterator::read_ntoh_u32 (void)
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
GBuffer::Iterator::read_ntoh_u64 (void)
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
GBuffer::Iterator::read (uint8_t *buffer, uint16_t size)
{
	assert (m_current + size <= m_end);
	memcpy (buffer, m_current, size);
	m_current += size;
}



}; // namespace yans


#endif /* GBUFFER_H */
