/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef WRITE_FILE_H
#define WRITE_FILE_H

#include <stdint.h>
#include <string>

class WriteFilePrivate;
class Host;

class WriteFile {
public:
	WriteFile (Host *host);
	WriteFile ();
	~WriteFile ();

	void open (std::string *filename);
	void close (void);

	ssize_t write (uint8_t *buf, size_t count);

private:
	WriteFilePrivate *m_priv;
};

#endif /* WRITE_FILE_H */

