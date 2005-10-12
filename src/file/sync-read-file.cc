/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "read-file.h"

ReadFile::ReadFile ()
	: m_file (0)
{}
ReadFile::~ReadFile ()
{
	if (m_file != 0) {
		fclose (m_file);
		m_file = 0;
	}
}

void
ReadFile::open (std::string *filename)
{
	m_file = fopen (filename->c_str (), "r");
	assert (m_file != 0);
}

ssize_t 
ReadFile::read (uint8_t *buf, size_t count)
{
	size_t bytes_read = fread (buf, 1, count, m_file);
	if (ferror (m_file)) {
		return -1;
	} else {
		return bytes_read;
	}
}
int 
ReadFile::scanf (const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int retval = vfscanf (m_file, format, ap);
	va_end(ap);
	return retval;
}
int 
ReadFile::vscanf (const char *format, va_list ap)
{
	int retval = vfscanf (m_file, format, ap);
	return retval;
}
