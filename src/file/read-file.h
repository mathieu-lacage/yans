/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef READ_FILE_H
#define READ_FILE_H

#include <stdint.h>
#include <string>

class ReadFilePrivate;

class ReadFile {
public:
	ReadFile ();
	~ReadFile ();

	void open (std::string *filename);
	void close (void);

	ssize_t read (uint8_t *buf, size_t count);
private:
	ReadFilePrivate *m_priv;
};

#endif /* READ_FILE_H */
