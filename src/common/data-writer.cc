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

#include "data-writer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <string.h>
#include <list>

#define noTRACE_DATA_WRITER 1

#ifdef TRACE_DATA_WRITER
#include <iostream>
# define TRACE(x) \
std::cout << "DATA WRITER TRACE " << this << " " << x << std::endl;
#else /* TRACE_DATA_WRITER */
# define TRACE(format,...)
#endif /* TRACE_DATA_WRITER */


#define BUFFER_SIZE (1<<12)
#define BUFFER_WRAP (BUFFER_SIZE<<1)

namespace yans {

class DataWriterPrivate {
public:
	DataWriterPrivate ();
	~DataWriterPrivate ();

	void open (char const *filename);
	void store (uint8_t *buffer, uint32_t size);
private:
	bool is_empty (void);
	bool is_full (void);
	uint32_t get_free (void);
	uint32_t get_used (void);
	uint32_t get_contiguous_free (void);
	uint32_t get_contiguous_used (void);
	uint8_t *get_current (void);
	void attempt_to_write_one_chunk (void);

	uint8_t m_data[BUFFER_SIZE];
	uint32_t m_read;
	uint32_t m_write;
	int m_fd;
};

DataWriterPrivate::DataWriterPrivate ()
{}
DataWriterPrivate::~DataWriterPrivate ()
{
	while (!is_empty ()) {
		attempt_to_write_one_chunk ();
	}
	::close (m_fd);
}


void
DataWriterPrivate::open (char const *filename)
{
	m_fd = ::open (filename, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, S_IRUSR | S_IWUSR);
	assert (m_fd != -1);
}

#define min(a,b) ((a)<(b)?(a):(b))

void
DataWriterPrivate::store (uint8_t *buffer, uint32_t size)
{
	TRACE ("store size="<<size);
	while (size > 0 ) {
		if (get_contiguous_free () > 0) {			
			uint32_t to_write = min (get_contiguous_free (), size);
			TRACE ("free="<<get_contiguous_free ()<<", to_write="<<to_write);
			memcpy (get_current (), buffer, to_write);
			size -= to_write;
			buffer += to_write;
			uint32_t new_write = m_write + to_write;
			new_write %= BUFFER_WRAP;
			m_write = new_write;
		} else {
			attempt_to_write_one_chunk ();
		}
	}
}

void
DataWriterPrivate::attempt_to_write_one_chunk (void)
{
	struct pollfd ufd;
	ufd.fd = m_fd;
	ufd.events = POLLOUT;
	ufd.revents = 0;
	poll (&ufd, 1, -1);
	uint8_t *current = &(m_data[m_read % BUFFER_SIZE]);
	TRACE ("write="<<get_contiguous_used ());
	ssize_t written = ::write (m_fd, current, get_contiguous_used ());
	assert (written != -1);
	uint32_t new_read = m_read + written;
	new_read %= BUFFER_WRAP;
	m_read = new_read;
}

bool
DataWriterPrivate::is_full (void)
{
	uint32_t write_mod = m_write % BUFFER_SIZE;
	uint32_t read_mod = m_read % BUFFER_SIZE;
	if (m_write != m_read &&
	    write_mod == read_mod) {
		return true;
	} else {
		return false;
	}
}

bool
DataWriterPrivate::is_empty (void)
{
	if (m_write == m_read) {
		return true;
	} else {
		return false;
	}
}

uint32_t
DataWriterPrivate::get_free (void)
{
	uint32_t free = 0;
	if (m_write >= m_read) {
		free += BUFFER_SIZE - m_write;
		free += m_read - 0;
	} else {
		free += m_read - m_write;
	}
	return free;
}
uint32_t
DataWriterPrivate::get_used (void)
{
	return BUFFER_SIZE - get_free ();
}
uint32_t
DataWriterPrivate::get_contiguous_free (void)
{
	uint32_t write_mod = m_write % BUFFER_SIZE;
	uint32_t read_mod = m_read % BUFFER_SIZE;
	if (m_write == m_read) {
		return BUFFER_SIZE - write_mod;
	} else if (write_mod == read_mod) {
		return 0;
	} else if (write_mod > read_mod) {
		return BUFFER_SIZE - write_mod;
	} else {
		return read_mod - write_mod;
	}
}
uint32_t
DataWriterPrivate::get_contiguous_used (void)
{
	uint32_t write_mod = m_write % BUFFER_SIZE;
	uint32_t read_mod = m_read % BUFFER_SIZE;
	if (m_write == m_read) {
		return 0;
	} else if (write_mod == read_mod) {
		return BUFFER_SIZE - read_mod;
	} else if (write_mod > read_mod) {
		return write_mod - read_mod;
	} else {
		return BUFFER_SIZE - read_mod;
	}
}
uint8_t *
DataWriterPrivate::get_current (void)
{
	uint8_t *current = &(m_data[m_write % BUFFER_SIZE]);
	return current;
}

DataWriter::DataWriter ()
	: m_priv (new DataWriterPrivate ())
{}
DataWriter::~DataWriter ()
{
	delete m_priv;
	m_priv = 0;
}

void 
DataWriter::open (char const *filename)
{
	m_priv->open (filename);
}
void 
DataWriter::write (uint8_t *buffer, uint32_t size)
{
	m_priv->store (buffer, size);
}

}; // namespace
