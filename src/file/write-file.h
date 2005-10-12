/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef WRITE_FILE_H
#define WRITE_FILE_H

#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>

class WriteFile {
public:
	WriteFile ();
	~WriteFile ();

	void open (std::string *filename);
	void close (void);

	ssize_t write (uint8_t *buf, size_t count);
	int printf (const char *format, ...);
	int vprintf (const char *format, va_list ap);

private:
	FILE *m_file;
};

#endif /* WRITE_FILE_H */

