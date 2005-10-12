/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "write-file.h"

WriteFile::WriteFile ()
	: m_file (0)
{}
WriteFile::~WriteFile ()
{
	if (m_file != 0) {
		fclose (m_file);
	}
}

void 
WriteFile::open (std::string *filename)
{
	m_file = fopen (filename->c_str (), "w");
}
void 
WriteFile::close (void)
{
	fclose (m_file);
}

ssize_t 
WriteFile::write (uint8_t *buf, size_t count)
{
	int retval = fwrite (buf, 1, count, m_file);
	if (ferror (m_file)) {
		return -1;
	} else {
		return retval;
	}
}

int 
WriteFile::printf (const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int retval = fprintf (m_file, format, ap);
	va_end(ap);
	return retval;
}
int 
WriteFile::vprintf (const char *format, va_list ap)
{
	int retval = vfprintf (m_file, format, ap);
	return retval;
}

