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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "runnable.h"
#include "host.h"
#include "read-file.h"

class ReadFilePrivate {
public:
	ReadFilePrivate () 
		: m_fd (-1) {}
	int m_fd;
};

ReadFile::ReadFile ()
	: m_priv (new ReadFilePrivate ())
{}
ReadFile::~ReadFile ()
{
	delete m_priv;
	m_priv = (ReadFilePrivate *)0xdeadbeaf;
}

void 
ReadFile::open (std::string *filename)
{
	std::string *file = new std::string (*(Runnable::get_host ()->m_root));
	file->append (*filename);
	m_priv->m_fd = ::open (filename->c_str (), O_RDONLY);
}
void 
ReadFile::close (void)
{
	::close (m_priv->m_fd);
	m_priv->m_fd = -1;
}

ssize_t 
ReadFile::read (uint8_t *buf, size_t count)
{
	return ::read (m_priv->m_fd, buf, count);
}
