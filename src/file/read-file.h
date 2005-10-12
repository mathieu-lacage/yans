/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef READ_FILE_H
#define READ_FILE_H

#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>

class ReadFile {
public:
	ReadFile ();
	~ReadFile ();

	void open (std::string *filename);
	void close (void);

	ssize_t read (uint8_t *buf, size_t count);
	int scanf (const char *format, ...);
	int vscanf (const char *format, va_list ap);

private:
	FILE *m_file;
};

#endif /* READ_FILE_H */
