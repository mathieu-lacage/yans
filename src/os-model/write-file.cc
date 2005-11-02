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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "runnable.h"
#include "host.h"
#include "write-file.h"

class WriteFilePrivate {
public:
	WriteFilePrivate ()
		: m_fd (-1), m_host (0) {}
	WriteFilePrivate (Host *host)
		: m_fd (-1), m_host (host) {}
	int m_fd;
	Host *m_host;
};

WriteFile::WriteFile (Host *host)
	: m_priv (new WriteFilePrivate (host))
{}
WriteFile::WriteFile ()
	: m_priv (new WriteFilePrivate ())
{}
WriteFile::~WriteFile ()
{
	delete m_priv;
	m_priv = (WriteFilePrivate *)0xdeadbeaf;
}
#include <errno.h>
#include <iostream>
void 
WriteFile::open (std::string *filename)
{	
	Host *host;
	if (m_priv->m_host == 0) {
		host = Runnable::get_host ();
	} else {
		host = m_priv->m_host;
	}
	std::string *root = new std::string (*(host->m_root));
	root->append (*filename);
	m_priv->m_fd = ::open (root->c_str (), O_WRONLY | O_CREAT, S_IRWXU);
	if (m_priv->m_fd == -1) {
		std::cerr << "errno: " << errno
			  << " err: " << strerror (errno) 
			  << " file: " << root->c_str ()
			  << std::endl;
	}
}
void 
WriteFile::close (void)
{
	::close (m_priv->m_fd);
	m_priv->m_fd = -1;
}

ssize_t 
WriteFile::write (uint8_t *buf, size_t count)
{
	ssize_t written = ::write (m_priv->m_fd, buf, count);
	assert (written >= 0);
	if (((size_t)written) != count) {
		std::cerr << "not wrote all: " << written << "/" << count << std::endl;
	} 
	return written;
}
