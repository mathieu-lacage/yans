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

#define FOO_INLINE

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
		FOO_INLINE Iterator ();
		FOO_INLINE Iterator (Iterator const&o);
		FOO_INLINE Iterator &operator = (Iterator const &o);
		/**
		 * go forward by one byte
		 */
		FOO_INLINE void next (void);
		/**
		 * go backward by one byte
		 */
		FOO_INLINE void prev (void);
		/**
		 * \param delta number of bytes to go forward
		 */
		FOO_INLINE void next (uint32_t delta);
		/**
		 * \param delta number of bytes to go backward
		 */
		FOO_INLINE void prev (uint32_t delta);
		/**
		 * \param o the second iterator
		 * \return number of bytes included between the two iterators
		 *
		 * This method works only if the two iterators point
		 * to the same underlying buffer. Debug builds ensure
		 * this with an assert.
		 */
		FOO_INLINE uint32_t get_distance_from (Iterator const &o) const;

		/**
		 * \return true if this iterator points to the end of the byte array.
		 *         false otherwise.
		 */
		FOO_INLINE bool is_end (void) const;
		/**
		 * \return true if this iterator points to the start of the byte array.
		 *         false otherwise.
		 */
		FOO_INLINE bool is_start (void) const;

		/**
		 * \return a pointer to the current position in the
		 *         internal buffer
		 *
		 * Please, try to never ever use this method. It is really
		 * evil and is present only for a few specific uses.
		 */
		FOO_INLINE uint8_t *peek_data (void);

		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by one byte.
		 */
		FOO_INLINE void write_u8 (uint8_t  data);
		/**
		 * \param data data to write in buffer
		 * \param len number of times data must be written in buffer
		 *
		 * Write the data in buffer len times and avance the iterator position
		 * by len byte.
		 */
		FOO_INLINE void write_u8 (uint8_t data, uint32_t len);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by two bytes. The format of the data written in the byte
		 * buffer is non-portable. We only ensure that read_u16 will
		 * return exactly what we wrote with write_u16 if the program
		 * is run on the same machine.
		 */
		FOO_INLINE void write_u16 (uint16_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by four bytes. The format of the data written in the byte
		 * buffer is non-portable. We only ensure that read_u32 will
		 * return exactly what we wrote with write_u32 if the program
		 * is run on the same machine.
		 */
		FOO_INLINE void write_u32 (uint32_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by eight bytes. The format of the data written in the byte
		 * buffer is non-portable. We only ensure that read_u64 will
		 * return exactly what we wrote with write_u64 if the program
		 * is run on the same machine.
		 */
		FOO_INLINE void write_u64 (uint64_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by two bytes. The data is written in network order and the
		 * input data is expected to be in host order.
		 */
		FOO_INLINE void write_hton_u16 (uint16_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by four bytes. The data is written in network order and the
		 * input data is expected to be in host order.
		 */
		FOO_INLINE void write_hton_u32 (uint32_t data);
		/**
		 * \param data data to write in buffer
		 *
		 * Write the data in buffer and avance the iterator position
		 * by eight bytes. The data is written in network order and the
		 * input data is expected to be in host order.
		 */
		FOO_INLINE void write_hton_u64 (uint64_t data);
		/**
		 * \param buffer a byte buffer to copy in the internal buffer.
		 * \param size number of bytes to copy.
		 *
		 * Write the data in buffer and avance the iterator position
		 * by size bytes.
		 */
		FOO_INLINE void write (uint8_t const*buffer, uint16_t size);
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
		FOO_INLINE void write (Iterator start, Iterator end);

		/**
		 * \return the byte read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 */
		FOO_INLINE uint8_t  read_u8 (void);
		/**
		 * \return the two bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in the format written by write_u16.
		 */
		FOO_INLINE uint16_t read_u16 (void);
		/**
		 * \return the four bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in the format written by write_u32.
		 */
		FOO_INLINE uint32_t read_u32 (void);
		/**
		 * \return the eight bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in the format written by write_u64.
		 */
		FOO_INLINE uint64_t read_u64 (void);
		/**
		 * \return the two bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in network format and return in host format.
		 */
		FOO_INLINE uint16_t read_ntoh_u16 (void);
		/**
		 * \return the four bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in network format and return in host format.
		 */
		FOO_INLINE uint32_t read_ntoh_u32 (void);
		/**
		 * \return the eight bytes read in the buffer.
		 *
		 * Read data and advance the Iterator by the number of bytes
		 * read.
		 * The data is read in network format and return in host format.
		 */
		FOO_INLINE uint64_t read_ntoh_u64 (void);
		/**
		 * \param buffer buffer to copy data into
		 * \param size number of bytes to copy
		 *
		 * Copy size bytes of data from the internal buffer to the
		 * input buffer and avance the Iterator by the number of
		 * bytes read.
		 */
		FOO_INLINE void read (uint8_t *buffer, uint16_t size);
	private:
		friend class Buffer;
		FOO_INLINE Iterator (uint8_t *start, uint8_t *end, uint8_t *current);
		uint8_t *m_start;
		uint8_t *m_end;
		uint8_t *m_current;
	};

	/**
	 * \return the number of bytes stored in this buffer.
	 */
	FOO_INLINE uint32_t get_size (void) const;

	/**
	 * \param start size to reserve
	 *
	 * Add bytes at the start of the Buffer. The
	 * content of these bytes is undefined but debugging
	 * builds initialize them to 0x33.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	FOO_INLINE void add_at_start (uint32_t start);
	/**
	 * \param end size to reserve
	 *
	 * Add bytes at the end of the Buffer. The
	 * content of these bytes is undefined but debugging
	 * builds initialize them to 0x33.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	FOO_INLINE void add_at_end (uint32_t end);
	/**
	 * \param start size to remove
	 *
	 * Remove bytes at the start of the Buffer.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	FOO_INLINE void remove_at_start (uint32_t start);
	/**
	 * \param end size to remove
	 *
	 * Remove bytes at the end of the Buffer.
	 * Any call to this method invalidates any Iterator
	 * pointing to this Buffer.
	 */
	FOO_INLINE void remove_at_end (uint32_t end);

	/**
	 * \return an Iterator which points to the
	 * start of this Buffer.
	 */
	FOO_INLINE Buffer::Iterator begin (void) const;
	/**
	 * \return an Iterator which points to the
	 * end of this Buffer.
	 */
	FOO_INLINE Buffer::Iterator end (void) const;

	FOO_INLINE Buffer (Buffer const &o);
	FOO_INLINE Buffer &operator = (Buffer const &o);
	FOO_INLINE Buffer ();
	FOO_INLINE ~Buffer ();
private:
	struct BufferData {
		uint32_t m_count;
		uint32_t m_size;
		uint32_t m_dirty_start;
		uint32_t m_dirty_size;
		uint8_t m_data;
	};
	typedef std::vector<struct Buffer::BufferData*> BufferDataList;

	FOO_INLINE uint8_t *get_start (void) const;
	void recycle (struct Buffer::BufferData *data);
	static struct Buffer::BufferData *create (void);
	static struct Buffer::BufferData *allocate (uint32_t size, uint32_t start);
	static void deallocate (struct Buffer::BufferData *data);

	static BufferDataList m_free_list;
	static uint32_t m_max_total_add_start;
	static uint32_t m_max_total_add_end;

	struct BufferData *m_data;
	uint32_t m_initial_start;
	uint32_t m_start;
	uint32_t m_size;
};

}; // namespace yans


#endif /* BUFFER_H */
