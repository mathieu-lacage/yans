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
#include <errno.h>
#include <iostream>


#include "host.h"
#include "write-file.h"

namespace yans {

class WriteFilePrivate {
public:
	WriteFilePrivate (Host *host)
		: m_fd (-1), m_host (host) {}
	int m_fd;
	Host *m_host;
};

WriteFile::WriteFile (Host *host)
	: m_priv (new WriteFilePrivate (host))
{}
WriteFile::~WriteFile ()
{
	delete m_priv;
	m_priv = (WriteFilePrivate *)0xdeadbeaf;
}
void 
WriteFile::open (std::string *filename)
{	
	std::string *root = new std::string (*(m_priv->m_host->m_root));
	root->append (*filename);
	m_priv->m_fd = ::open (root->c_str (), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	if (m_priv->m_fd == -1) {
		std::cerr << "errno: " << errno
			  << " err: " << strerror (errno) 
			  << " file: " << root->c_str ()
			  << std::endl;
	}
	delete root;
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
	size_t written = 0;
	while (written < count) {
	       ssize_t tmp = ::write (m_priv->m_fd, buf+written, count-written);
	       assert (tmp >= 0);
	       written += tmp;
	}
	return written;
}

}; // namespace yans
