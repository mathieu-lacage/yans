/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

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
	m_priv->m_fd = ::open (root->c_str (), O_WRONLY);
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
	return ::write (m_priv->m_fd, buf, count);
}
