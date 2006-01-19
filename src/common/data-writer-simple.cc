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


namespace yans {

class DataWriterPrivate {
public:
	DataWriterPrivate ();
	~DataWriterPrivate ();

	void open (char const *filename);
	void write (uint8_t *buffer, uint32_t size);
private:
	int m_fd;
};

DataWriterPrivate::DataWriterPrivate ()
{}
DataWriterPrivate::~DataWriterPrivate ()
{
	::close (m_fd);
}


void
DataWriterPrivate::open (char const *filename)
{
	m_fd = ::open (filename, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, S_IRUSR | S_IWUSR);
	assert (m_fd != -1);
}

void
DataWriterPrivate::write (uint8_t *buffer, uint32_t size)
{
	::write (m_fd, buffer, size);
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
	m_priv->write (buffer, size);
}

}; // namespace
