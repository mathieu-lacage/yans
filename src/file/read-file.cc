/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
