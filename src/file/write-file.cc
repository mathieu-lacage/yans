/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "write-file.h"

class WriteFilePrivate {
public:
	WriteFilePrivate ()
		: m_fd (-1) {}
	int m_fd;
};

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
	m_priv->m_fd = ::open (filename->c_str (), O_WRONLY);
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
